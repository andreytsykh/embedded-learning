/*
 * LDR + ADC oneshot + SMA + hysteresis LED control
 *
 * Circuit used:
 *
 *     3.3V --- LDR --- ADC --- 10k resistor --- GND
 *
 * For this circuit:
 *     Light  => RAW value becomes bigger
 *     Dark   => RAW value becomes smaller
 */

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "LDR_SMA";

/*---------------------------------------------------------------
        ADC / GPIO configuration
---------------------------------------------------------------*/

#define ADC1_CHAN0              ADC_CHANNEL_3
#define ADC_ATTEN               ADC_ATTEN_DB_12
#define ADC_BITWIDTH            ADC_BITWIDTH_12

#define LED_GPIO                GPIO_NUM_7
#define LED_ACTIVE_LEVEL        1

#define MEASUREMENT_DELAY_MS    100U

#define ADC_FULL_SCALE_MV       3300.0f
#define ADC_MAX_RAW             4095.0f



#define SMA_WINDOW_SIZE         10U
#define DARK_RAW_THRESHOLD      800
#define LIGHT_RAW_THRESHOLD     1100

static bool adc_calibration_init(
    adc_unit_t unit,
    adc_channel_t channel,
    adc_atten_t atten,
    adc_cali_handle_t *out_handle
);

static void led_set(bool on)
{
    const uint32_t level = on ? LED_ACTIVE_LEVEL : (1U - LED_ACTIVE_LEVEL);
    gpio_set_level(LED_GPIO, (uint32_t)level);
}

void app_main(void)
{
    const gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&led_cfg));
    led_set(false);

    adc_oneshot_unit_handle_t adc1_handle = NULL;

    const adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    const adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc1_handle,
        ADC1_CHAN0,
        &config
    ));

    adc_cali_handle_t adc1_cali_chan0_handle = NULL;

    const bool do_calibration1_chan0 = adc_calibration_init(
        ADC_UNIT_1,
        ADC1_CHAN0,
        ADC_ATTEN,
        &adc1_cali_chan0_handle
    );

    int32_t sma_buffer[SMA_WINDOW_SIZE] = {0};
    uint32_t sma_index = 0U;
    uint32_t sma_count = 0U;
    int32_t sma_sum = 0;

    bool led_on = false;

    ESP_LOGI(TAG, "LDR ADC oneshot + SMA + hysteresis started");
    ESP_LOGI(TAG, "Circuit: 3.3V --- LDR --- ADC --- 10k resistor --- GND");
    ESP_LOGI(TAG, "ADC channel=%d, measurement period=%" PRIu32 " ms", ADC1_CHAN0, (uint32_t)MEASUREMENT_DELAY_MS);
    ESP_LOGI(TAG, "SMA window=%" PRIu32 ", dark threshold=%d, light threshold=%d, LED GPIO=%d",
             (uint32_t)SMA_WINDOW_SIZE,
             DARK_RAW_THRESHOLD,
             LIGHT_RAW_THRESHOLD,
             LED_GPIO);

    while (true) {
        int raw = 0;
        int cali_mv = 0;

        ESP_ERROR_CHECK(adc_oneshot_read(
            adc1_handle,
            ADC1_CHAN0,
            &raw
        ));

        sma_sum -= sma_buffer[sma_index];
        sma_buffer[sma_index] = raw;
        sma_sum += raw;

        sma_index = (sma_index + 1U) % SMA_WINDOW_SIZE;

        if (sma_count < SMA_WINDOW_SIZE) {
            sma_count++;
        }

        const int32_t sma_raw = sma_sum / (int32_t)sma_count;
        const float manual_mv = ((float)raw * ADC_FULL_SCALE_MV) / ADC_MAX_RAW;

        if (do_calibration1_chan0) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
                adc1_cali_chan0_handle,
                raw,
                &cali_mv
            ));
        }

        if ((!led_on) && (sma_raw <= DARK_RAW_THRESHOLD)) {
            led_on = true;
            led_set(true);
        } else if (led_on && (sma_raw >= LIGHT_RAW_THRESHOLD)) {
            led_on = false;
            led_set(false);
        }

        if (do_calibration1_chan0) {
            ESP_LOGI(TAG,
                     "RAW=%" PRId32 " SMA=%" PRId32 " U_MANUAL=%.1f mV U_CALI=%" PRId32 " mV LED=%s",
                     raw,
                     sma_raw,
                     (double)manual_mv,
                     cali_mv,
                     led_on ? "ON" : "OFF");
        } else {
            ESP_LOGI(TAG,
                     "RAW=%" PRId32 " SMA=%" PRId32 " U_MANUAL=%.1f mV U_CALI=N/A LED=%s",
                     raw,
                     sma_raw,
                     (double)manual_mv,
                     led_on ? "ON" : "OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_DELAY_MS));
    }
}


static bool adc_calibration_init(
    adc_unit_t unit,
    adc_channel_t channel,
    adc_atten_t atten,
    adc_cali_handle_t *out_handle
)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "Calibration scheme: Curve Fitting");

    const adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH,
    };

    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);

    if (ret == ESP_OK) {
        calibrated = true;
    }
#endif

    *out_handle = handle;

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration success");
    } else if ((ret == ESP_ERR_NOT_SUPPORTED) || (!calibrated)) {
        ESP_LOGW(TAG, "Calibration not supported or eFuse data is not available");
    } else {
        ESP_LOGE(TAG, "Calibration failed: invalid argument or no memory");
    }

    return calibrated;
}
