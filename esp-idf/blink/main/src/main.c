#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"


#define LED_GPIO        GPIO_NUM_4
#define BOOT_GPIO       GPIO_NUM_0

// GPIO8 
#define LDR_ADC_CHANNEL ADC_CHANNEL_7


// PWM
#define PWM_MODE        LEDC_LOW_SPEED_MODE
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_CHANNEL     LEDC_CHANNEL_0
#define PWM_FREQUENCY   1000
#define PWM_RESOLUTION  LEDC_TIMER_10_BIT
#define PWM_MAX         1023


// Setpoint
#define SETPOINT_START  500.0f
#define SETPOINT_STEP   500.0f
#define SETPOINT_MAX    4000.0f


// PID
static float PID_KP = 0.35f;
static float PID_KI = 0.16f;
static float PID_KD = 0.0f;

static float setpoint = SETPOINT_START;

static const char *TAG = "light_pid";


static int read_ldr_average(
    adc_oneshot_unit_handle_t adc_handle
)
{
    int sum = 0;

    for (int i = 0; i < 5; i++) {
        int value = 0;

        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                LDR_ADC_CHANNEL,
                &value
            )
        );

        sum += value;
    }

    return sum / 5;
}


static void set_led_pwm(uint32_t duty)
{
    if (duty > PWM_MAX) {
        duty = PWM_MAX;
    }

    ESP_ERROR_CHECK(
        ledc_set_duty(
            PWM_MODE,
            PWM_CHANNEL,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            PWM_MODE,
            PWM_CHANNEL
        )
    );
}


static void setup_pwm(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = PWM_MODE,
        .timer_num = PWM_TIMER,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );

    const ledc_channel_config_t channel_config = {
        .gpio_num = LED_GPIO,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );

    set_led_pwm(0);
}


static adc_oneshot_unit_handle_t setup_adc(void)
{
    adc_oneshot_unit_handle_t adc_handle;

    const adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &adc_init_config,
            &adc_handle
        )
    );

    const adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            LDR_ADC_CHANNEL,
            &adc_channel_config
        )
    );

    return adc_handle;
}


static void setup_boot_button(void)
{
    const gpio_config_t boot_config = {
        .pin_bit_mask = 1ULL << BOOT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&boot_config)
    );
}


static void process_boot_button(
    bool *button_ready,
    float *integral,
    float *previous_error
)
{
    int button_level = gpio_get_level(BOOT_GPIO);

    if (!*button_ready && button_level == 1) {
        *button_ready = true;
    }

    if (*button_ready && button_level == 0) {
        vTaskDelay(pdMS_TO_TICKS(30));

        if (gpio_get_level(BOOT_GPIO) == 0) {
            *button_ready = false;

            setpoint += SETPOINT_STEP;

            if (setpoint > SETPOINT_MAX) {
                setpoint = SETPOINT_START;
            }

            *integral = 0.0f;
            *previous_error = 0.0f;

            ESP_LOGI(
                TAG,
                "New setpoint: %.0f",
                setpoint
            );

            while (gpio_get_level(BOOT_GPIO) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }
}


void app_main(void)
{
    setup_pwm();
    setup_boot_button();

    adc_oneshot_unit_handle_t adc_handle =
        setup_adc();

    bool button_ready = false;

    float integral = 0.0f;
    float previous_error = 0.0f;

    int64_t previous_time_us =
        esp_timer_get_time();

    ESP_LOGI(TAG, "Light PID controller started");
    ESP_LOGI(TAG, "Initial setpoint: %.0f", setpoint);

    while (true) {
        process_boot_button(
            &button_ready,
            &integral,
            &previous_error
        );

        int64_t current_time_us =
            esp_timer_get_time();

        float dt =
            (float)(current_time_us - previous_time_us)
            / 1000000.0f;

        previous_time_us = current_time_us;

        if (dt <= 0.0f || dt > 0.5f) {
            dt = 0.05f;
        }

        int measured_value =
            read_ldr_average(adc_handle);

        float error =
            setpoint - (float) measured_value;

        integral += error * dt;

        if (integral > 10000.0f) {
            integral = 10000.0f;
        }

        if (integral < -10000.0f) {
            integral = -10000.0f;
        }

        float derivative =
            (error - previous_error) / dt;

        float p_term = PID_KP * error;
        float i_term = PID_KI * integral;
        float d_term = PID_KD * derivative;

        float output =
            p_term + i_term + d_term;

        previous_error = error;

        if (output < 0.0f) {
            output = 0.0f;
        }

        if (output > PWM_MAX) {
            output = PWM_MAX;
        }

        uint32_t duty =
            (uint32_t) output;

        set_led_pwm(duty);

        printf(
            "setpoint=%.0f "
            "measured=%d "
            "error=%.1f "
            "dt=%.4f "
            "P=%.1f "
            "I=%.1f "
            "D=%.1f "
            "pwm=%lu\n",

            setpoint,
            measured_value,
            error,
            dt,
            p_term,
            i_term,
            d_term,
            (unsigned long) duty
        );

     
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}