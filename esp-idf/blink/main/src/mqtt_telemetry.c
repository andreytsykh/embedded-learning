#include "mqtt_telemetry.h"
#include "app_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

static atomic_bool wifi_ready;
static atomic_bool mqtt_ready;
static esp_mqtt_client_handle_t client;
static QueueHandle_t display_queue;

#define SOURCE_COUNT 8
static telemetry_view_t sources[SOURCE_COUNT];
static unsigned next_source;

static void process_telemetry(char *topic, char *text)
{
    const char prefix[] = APP_MQTT_GROUP "/";
    if (strncmp(topic, prefix, sizeof(prefix) - 1)) return;
    char *source = topic + sizeof(prefix) - 1;
    char *field = strchr(source, '/');
    if (!field) return;
    *field++ = '\0';
    if (!source[0] || strlen(source) >= sizeof(sources[0].source)) return;
    bool t = strcmp(field, "temp") == 0;
    bool h = strcmp(field, "hum") == 0;
    bool p = strcmp(field, "pres") == 0;
    if (!t && !h && !p) return;
    char *end;
    float value = strtof(text, &end);
    if (end == text || !isfinite(value)) return;
    while (*end == ' ' || *end == '\r' || *end == '\n' || *end == '\t') end++;
    if (*end) return;
   
    if (t && (value < -40 || value > 85)) return;
    if (h && (value < 0 || value > 100)) return;
    if (p && (value < 300 || value > 1100)) return;
    telemetry_view_t *view = NULL;
    for (int i = 0; i < SOURCE_COUNT; i++) {
        if (strcmp(sources[i].source, source) == 0) {
            view = &sources[i];
            break;
        }
    }
    if (!view) {
        view = &sources[next_source];
        next_source = (next_source + 1) % SOURCE_COUNT;
        *view = (telemetry_view_t){
            .temperature = NAN, .humidity = NAN, .pressure = NAN,
        };
        strcpy(view->source, source);
    }
    if (t) view->temperature = value;
    if (h) view->humidity = value;
    if (p) view->pressure = value;
    xQueueOverwrite(display_queue, view);
}

static void receive_telemetry(esp_mqtt_event_handle_t event)
{
    static char topic[64], text[32];
    static int received, total;
    static bool receiving;
    if (event->current_data_offset == 0) {
        receiving = false;
        received = 0;
        if (event->topic_len <= 0 || event->topic_len >= (int)sizeof(topic) ||
            event->total_data_len <= 0 || event->total_data_len >= (int)sizeof(text)) return;
        memcpy(topic, event->topic, event->topic_len);
        topic[event->topic_len] = '\0';
        total = event->total_data_len;
        receiving = true;
    }
    if (!receiving) return;
    if (event->current_data_offset != received || event->total_data_len != total ||
        event->data_len < 0 || event->data_len > total - received) {
        receiving = false;
        return;
    }
    if (event->data_len > 0) memcpy(text + received, event->data, event->data_len);
    received += event->data_len;
    if (received == total) {
        text[received] = '\0';
        receiving = false;
        process_telemetry(topic, text);
    }
}

bool mqtt_telemetry_get_view(telemetry_view_t *out)
{
    return out && display_queue && xQueueReceive(display_queue, out, 0) == pdTRUE;
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_ready = false;
            mqtt_ready = false;
        }
        if (id == WIFI_EVENT_STA_START || id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_err_t err = esp_wifi_connect();
            if (err != ESP_OK) printf("Wi-Fi connect: %s\n", esp_err_to_name(err));
        }
    } else if (base == IP_EVENT) {
        if (id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = data;
            wifi_ready = true;
            printf("Wi-Fi IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        } else if (id == IP_EVENT_STA_LOST_IP) {
            wifi_ready = false;
        }
    }
}

static void mqtt_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    (void)base;
    esp_mqtt_event_handle_t event = data;
    if (id == MQTT_EVENT_CONNECTED) {
        mqtt_ready = true;
        printf("MQTT connected\n");
        if (esp_mqtt_client_subscribe(event->client, APP_SUBSCRIBE_TOPIC, 1) < 0)
            printf("Subscription request failed\n");
    } else if (id == MQTT_EVENT_DISCONNECTED) {
        mqtt_ready = false;
        printf("MQTT disconnected\n");
    } else if (id == MQTT_EVENT_SUBSCRIBED) {
        printf("Subscription acknowledged: %s\n", APP_SUBSCRIBE_TOPIC);
    } else if (id == MQTT_EVENT_DATA) {
        receive_telemetry(event);
    } else if (id == MQTT_EVENT_ERROR) {
        printf("MQTT error\n");
    }
}

void mqtt_telemetry_start(void)
{
    display_queue = xQueueCreate(1, sizeof(telemetry_view_t));
    if (!display_queue) abort();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    if (!esp_netif_create_default_wifi_sta()) abort();
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, wifi_event, NULL));
    wifi_config_t wifi = {.sta = {.ssid = APP_WIFI_SSID, .password = APP_WIFI_PASSWORD}};
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi));
    ESP_ERROR_CHECK(esp_wifi_start());
    while (!wifi_ready) vTaskDelay(pdMS_TO_TICKS(100));
    esp_mqtt_client_config_t mqtt = {
        .broker.address.uri = APP_MQTT_URI,
        .credentials.username = APP_MQTT_USERNAME[0] ? APP_MQTT_USERNAME : NULL,
        .credentials.authentication.password = APP_MQTT_PASSWORD[0] ? APP_MQTT_PASSWORD : NULL,
        .network.reconnect_timeout_ms = 3000,
        .outbox.limit = 4096,
    };
    client = esp_mqtt_client_init(&mqtt);
    if (!client) abort();
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

bool mqtt_telemetry_connected(void)
{
    return wifi_ready && mqtt_ready;
}

static bool publish_number(const char *topic, double value)
{
    char text[32];
    int len = snprintf(text, sizeof(text), "%.2f", value);
    if (len <= 0 || len >= (int)sizeof(text)) return false;
    return esp_mqtt_client_enqueue(client, topic, text, len, 1, 0, true) >= 0;
}

esp_err_t mqtt_telemetry_publish(const bme280_reading_t *data)
{
    if (!data) return ESP_ERR_INVALID_ARG;
    if (!mqtt_telemetry_connected()) return ESP_ERR_INVALID_STATE;
    bool t = publish_number(APP_TOPIC_TEMP, data->temperature_c);
    bool h = publish_number(APP_TOPIC_HUM, data->humidity_pct);
    bool p = publish_number(APP_TOPIC_PRES, data->pressure_hpa);
    return t && h && p ? ESP_OK : ESP_FAIL;
}
