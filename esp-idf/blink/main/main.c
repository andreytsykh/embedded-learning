#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "POT_LED_MOTOR"

#define ADC_CHANNEL     ADC_CHANNEL_6
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_12

#define MOTOR_GPIO      4
#define LED_GPIO        6

#define PWM_FREQ_HZ     5000U
#define PWM_RESOLUTION  LEDC_TIMER_12_BIT
#define PWM_MAX_DUTY    4095

#define ADC_DEAD_ZONE   50U
#define ADC_SAMPLES     16
#define DUTY_HYSTERESIS 10U

static void set_pwm_duty(ledc_channel_t channel, uint32_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channel));
}

static int adc_read_avg(adc_oneshot_unit_handle_t handle, adc_channel_t ch)
{
    int sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        int val = 0;
        adc_oneshot_read(handle, ch, &val);
        sum += val;
    }
    return sum / ADC_SAMPLES;
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;
    uint32_t last_duty = 0U;

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz         = PWM_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t motor_pwm_cfg = {
        .gpio_num   = MOTOR_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0U,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&motor_pwm_cfg));

    ledc_channel_config_t led_pwm_cfg = {
        .gpio_num   = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0U,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&led_pwm_cfg));

    adc_oneshot_unit_init_cfg_t adc_unit_cfg = {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t adc_chan_cfg = {
        .bitwidth = ADC_BITWIDTH,
        .atten    = ADC_ATTEN
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,
                                               ADC_CHANNEL,
                                               &adc_chan_cfg));

    while (true)
    {
        int raw = adc_read_avg(adc_handle, ADC_CHANNEL);

        uint32_t duty = 0U;

        if ((uint32_t)raw > ADC_DEAD_ZONE)
        {
            duty = (uint32_t)raw - ADC_DEAD_ZONE;
        }

        if (duty > PWM_MAX_DUTY)
        {
            duty = PWM_MAX_DUTY;
        }

        if (duty > last_duty + DUTY_HYSTERESIS ||
            (last_duty >= DUTY_HYSTERESIS && duty < last_duty - DUTY_HYSTERESIS) ||
            (last_duty < DUTY_HYSTERESIS && duty == 0U))
        {
            set_pwm_duty(LEDC_CHANNEL_0, duty);
            set_pwm_duty(LEDC_CHANNEL_1, duty);
            last_duty = duty;

            ESP_LOGI(TAG,
                     "raw=%d motor_duty=%lu led_duty=%lu",
                     raw,
                     (unsigned long)duty,
                     (unsigned long)duty);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}