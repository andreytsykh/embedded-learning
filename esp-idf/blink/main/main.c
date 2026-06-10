/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/soc_caps.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "EXAMPLE";

/*---------------------------------------------------------------
        ADC General Macros
---------------------------------------------------------------*/

#define ADC1_CHAN0              ADC_CHANNEL_3
#define ADC_ATTEN               ADC_ATTEN_DB_12
#define ADC_BITWIDTH            ADC_BITWIDTH_12

#define ADC_VREF_MV             1100
#define ADC_FULL_SCALE_MV       3300.0f
#define ADC_MAX_RAW             4095
#define MEASUREMENT_DELAY_MS    100

static bool adc_calibration_init(
    adc_unit_t unit,
    adc_channel_t channel,
    adc_atten_t atten,
    adc_cali_handle_t *out_handle
);

void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc1_handle,
        ADC1_CHAN0,
        &config
    ));

    adc_cali_handle_t adc1_cali_chan0_handle = NULL;

    bool do_calibration1_chan0 = adc_calibration_init(
        ADC_UNIT_1,
        ADC1_CHAN0,
        ADC_ATTEN,
        &adc1_cali_chan0_handle
    );

    printf("\nADC table output\n");
    printf("Channel: ADC1_CH%d | Vref: %d mV | Atten: ADC_ATTEN_DB_12) | Bitwidth: 12 bit | Period: %d ms\n",
           ADC1_CHAN0,
           ADC_VREF_MV,
           MEASUREMENT_DELAY_MS);
    printf("RAW   U_manual(mV)   U_cali(mV)   Error(%%)\n");
    printf("------------------------------------------\n");

    while (1) {
        int raw = 0;
        int cali_mv = 0;

        ESP_ERROR_CHECK(adc_oneshot_read(
            adc1_handle,
            ADC1_CHAN0,
            &raw
        ));

        float manual_mv = (float)raw * ADC_FULL_SCALE_MV / ADC_MAX_RAW;

        if (do_calibration1_chan0) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
                adc1_cali_chan0_handle,
                raw,
                &cali_mv
            ));

            float error_percent = 0.0f;

            if (cali_mv != 0) {
                error_percent = fabsf(manual_mv - cali_mv) * 100.0f / cali_mv;
            }

            printf("%4d   %11.1f   %10d   %8.2f\n",
                   raw,
                   manual_mv,
                   cali_mv,
                   error_percent);
        } else {
            printf("%4d   %11.1f   %10s   %8s\n",
                   raw,
                   manual_mv,
                   "N/A",
                   "N/A");
        }

        vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_DELAY_MS));
    }
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/

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
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");

        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };

        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);

        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}
