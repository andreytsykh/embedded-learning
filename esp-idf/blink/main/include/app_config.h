#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/gpio.h"

#define APP_I2C_SDA GPIO_NUM_8
#define APP_I2C_SCL GPIO_NUM_9

#define APP_WIFI_SSID       ""
#define APP_WIFI_PASSWORD   ""


#define APP_MQTT_URI        "mqtt://192.168.31.115:1883"
#define APP_MQTT_USERNAME   ""  
#define APP_MQTT_PASSWORD   ""

#define APP_MQTT_GROUP "EMB6"
#define APP_MQTT_OWNER "atsykhmeistruk"
#define APP_TOPIC_PREFIX APP_MQTT_GROUP "/" APP_MQTT_OWNER
#define APP_TOPIC_TEMP      APP_TOPIC_PREFIX "/temp"
#define APP_TOPIC_HUM       APP_TOPIC_PREFIX "/hum"
#define APP_TOPIC_PRES      APP_TOPIC_PREFIX "/pres"

#define APP_SAMPLE_PERIOD_MS 500
#define APP_SUBSCRIBE_TOPIC APP_MQTT_GROUP "/#"

#define APP_I2C_SDA GPIO_NUM_8
#define APP_I2C_SCL GPIO_NUM_9
#define APP_OLED_ADDRESS 0x3C
#endif
