#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_err.h"
#include "esp_log.h"

#define LDR_ADC_CHANNEL            ADC_CHANNEL_4
#define LDR_ADC_UNIT               ADC_UNIT_1
#define LDR_ADC_ATTEN              ADC_ATTEN_DB_12
#define LDR_ADC_BITWIDTH           ADC_BITWIDTH_12

#define SERVO_GPIO                 4

#define SERVO_FREQUENCY_HZ         50U
#define SERVO_PERIOD_US            20000U

#define SERVO_TIMER                LEDC_TIMER_0
#define SERVO_CHANNEL              LEDC_CHANNEL_0
#define SERVO_SPEED_MODE           LEDC_LOW_SPEED_MODE

#define SERVO_RESOLUTION           LEDC_TIMER_12_BIT
#define SERVO_RESOLUTION_BITS      12U
#define SERVO_DUTY_STEPS           4096U
#define SERVO_MAX_DUTY             4095U

#define SERVO_MIN_PULSE_US         500U
#define SERVO_MAX_PULSE_US         2500U

#define SERVO_MIN_ANGLE_DEG        10U
#define SERVO_MAX_ANGLE_DEG        170U

#define MEDIAN_WINDOW_SIZE         5U

#define LUT_HYSTERESIS_MV          100U

#define SERVO_STEP_DEG             1U
#define SUPERLOOP_DELAY_MS         20U

typedef struct
{
    uint32_t values[MEDIAN_WINDOW_SIZE];
    size_t write_index;
    size_t count;
} median_filter_t;

typedef struct
{
    uint32_t max_voltage_mv;
    uint32_t servo_angle_deg;
} light_servo_lut_entry_t;

static const light_servo_lut_entry_t light_servo_lut[] = {
    { 500,  10U  },
    { 800, 45U  },
    { 1200, 90U },
    { 2300, 135U },
    { 3141, 170U }
};

#define LIGHT_SERVO_LUT_SIZE \
    (sizeof(light_servo_lut) / sizeof(light_servo_lut[0]))

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t calibration_handle = NULL;

static uint32_t current_servo_angle = SERVO_MIN_ANGLE_DEG;
static uint32_t target_servo_angle = SERVO_MIN_ANGLE_DEG;

static const char *TAG = "MINI_PROJECT";

static uint32_t clamp_u32(
    uint32_t value,
    uint32_t minimum,
    uint32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

static int compare_u32(const void *left_pointer, const void *right_pointer)
{
    const uint32_t left = *(const uint32_t *)left_pointer;
    const uint32_t right = *(const uint32_t *)right_pointer;

    if (left < right) {
        return -1;
    }

    if (left > right) {
        return 1;
    }

    return 0;
}

static uint32_t median_filter_update(
    median_filter_t *filter,
    uint32_t new_value)
{
    uint32_t sorted_values[MEDIAN_WINDOW_SIZE];

    if (filter == NULL) {
        return new_value;
    }

    filter->values[filter->write_index] = new_value;

    filter->write_index =
        (filter->write_index + 1U) % MEDIAN_WINDOW_SIZE;

    if (filter->count < MEDIAN_WINDOW_SIZE) {
        ++filter->count;
    }

    for (size_t index = 0U; index < filter->count; ++index) {
        sorted_values[index] = filter->values[index];
    }

    qsort(
        sorted_values,
        filter->count,
        sizeof(sorted_values[0]),
        compare_u32
    );

    return sorted_values[filter->count / 2U];
}

static void ldr_init(void)
{
    const adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = LDR_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &unit_config,
            &adc_handle
        )
    );

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = LDR_ADC_ATTEN,
        .bitwidth = LDR_ADC_BITWIDTH
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            LDR_ADC_CHANNEL,
            &channel_config
        )
    );

    const adc_cali_curve_fitting_config_t calibration_config = {
        .unit_id = LDR_ADC_UNIT,
        .chan = LDR_ADC_CHANNEL,
        .atten = LDR_ADC_ATTEN,
        .bitwidth = LDR_ADC_BITWIDTH
    };

    ESP_ERROR_CHECK(
        adc_cali_create_scheme_curve_fitting(
            &calibration_config,
            &calibration_handle
        )
    );

    ESP_LOGI(TAG, "LDR ADC initialized");
    ESP_LOGI(TAG, "ADC calibration enabled");
}

static esp_err_t ldr_read(int *raw, int *voltage_mv)
{

    esp_err_t result = adc_oneshot_read(
        adc_handle,
        LDR_ADC_CHANNEL,
        raw
    );

    if (result != ESP_OK) {
        return result;
    }

    return adc_cali_raw_to_voltage(
        calibration_handle,
        *raw,
        voltage_mv
    );
}

static uint32_t servo_angle_to_pulse_us(uint32_t angle_deg)
{
    angle_deg = clamp_u32(
        angle_deg,
        SERVO_MIN_ANGLE_DEG,
        SERVO_MAX_ANGLE_DEG
    );

    const uint32_t angle_range =
        SERVO_MAX_ANGLE_DEG - SERVO_MIN_ANGLE_DEG;

    const uint32_t pulse_range =
        SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US;

    const uint32_t relative_angle =
        angle_deg - SERVO_MIN_ANGLE_DEG;

    return SERVO_MIN_PULSE_US +
           (uint32_t)(
               ((uint64_t)relative_angle * pulse_range) /
               angle_range
           );
}

static uint32_t servo_pulse_us_to_duty(uint32_t pulse_us)
{
    pulse_us = clamp_u32(
        pulse_us,
        SERVO_MIN_PULSE_US,
        SERVO_MAX_PULSE_US
    );

    uint32_t duty = (uint32_t)(
        (pulse_us * SERVO_DUTY_STEPS) /
        SERVO_PERIOD_US
    );

    if (duty > SERVO_MAX_DUTY) {
        duty = SERVO_MAX_DUTY;
    }

    return duty;
}

static void servo_set_angle(uint32_t angle_deg)
{
    const uint32_t pulse_us =
        servo_angle_to_pulse_us(angle_deg);

    const uint32_t duty =
        servo_pulse_us_to_duty(pulse_us);

    ESP_ERROR_CHECK(
        ledc_set_duty(
            SERVO_SPEED_MODE,
            SERVO_CHANNEL,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            SERVO_SPEED_MODE,
            SERVO_CHANNEL
        )
    );

    ESP_LOGI(
        TAG,
        "Servo: angle=%lu deg, pulse=%lu us, duty=%lu",
        (unsigned long)angle_deg,
        (unsigned long)pulse_us,
        (unsigned long)duty
    );
}

static void servo_update_smooth(void)
{
    uint32_t difference;
    uint32_t step;

    if (current_servo_angle < target_servo_angle) {
        difference = target_servo_angle - current_servo_angle;
        step = (difference < SERVO_STEP_DEG)
            ? difference
            : SERVO_STEP_DEG;

        current_servo_angle += step;
        servo_set_angle(current_servo_angle);
    } else if (current_servo_angle > target_servo_angle) {
        difference = current_servo_angle - target_servo_angle;
        step = (difference < SERVO_STEP_DEG)
            ? difference
            : SERVO_STEP_DEG;

        current_servo_angle -= step;
        servo_set_angle(current_servo_angle);
    }
}

static void servo_init(void)
{
    const ledc_timer_config_t timer_config = {
        .speed_mode = SERVO_SPEED_MODE,
        .duty_resolution = SERVO_RESOLUTION,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );

    const uint32_t initial_angle_deg =
        light_servo_lut[0].servo_angle_deg;

    const uint32_t initial_pulse_us =
        servo_angle_to_pulse_us(initial_angle_deg);

    const ledc_channel_config_t channel_config = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_SPEED_MODE,
        .channel = SERVO_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = SERVO_TIMER,
        .duty = servo_pulse_us_to_duty(initial_pulse_us),
        .hpoint = 0U,
        .flags = {
            .output_invert = 0U
        }
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );

    ESP_LOGI(TAG, "Servo initialized on GPIO%d", SERVO_GPIO);
}

static size_t light_lut_find_index(uint32_t voltage_mv)
{
    for (size_t index = 0U;
         index < LIGHT_SERVO_LUT_SIZE;
         ++index) {

        if (voltage_mv <=
            light_servo_lut[index].max_voltage_mv) {

            return index;
        }
    }

    return LIGHT_SERVO_LUT_SIZE - 1U;
}

static size_t light_lut_update_index(
    uint32_t voltage_mv,
    size_t current_index)
{
    if (current_index >= LIGHT_SERVO_LUT_SIZE) {
        return light_lut_find_index(voltage_mv);
    }

    while ((current_index + 1U) < LIGHT_SERVO_LUT_SIZE) {
        const uint32_t upper_transition_mv =
            light_servo_lut[current_index].max_voltage_mv +
            LUT_HYSTERESIS_MV;

        if (voltage_mv > upper_transition_mv) {
            ++current_index;
        } else {
            break;
        }
    }

    while (current_index > 0U) {
        const uint32_t previous_limit_mv =
            light_servo_lut[current_index - 1U].max_voltage_mv;

        const uint32_t lower_transition_mv =
            (previous_limit_mv > LUT_HYSTERESIS_MV)
                ? previous_limit_mv - LUT_HYSTERESIS_MV
                : 0U;

        if (voltage_mv < lower_transition_mv) {
            --current_index;
        } else {
            break;
        }
    }

    return current_index;
}

void app_main(void)
{
    median_filter_t median_filter = {
        .values = { 0 },
        .write_index = 0U,
        .count = 0U
    };

    bool lut_initialized = false;
    size_t current_lut_index = 0U;

    ldr_init();
    servo_init();

    current_servo_angle = SERVO_MIN_ANGLE_DEG;
    target_servo_angle = SERVO_MIN_ANGLE_DEG;

    while (true) {
        int raw = 0;
        int voltage_mv = 0;

        const esp_err_t read_result = ldr_read(
            &raw,
            &voltage_mv
        );

        if (read_result != ESP_OK) {
            ESP_LOGE(
                TAG,
                "LDR read failed: %s",
                esp_err_to_name(read_result)
            );

            vTaskDelay(
                pdMS_TO_TICKS(SUPERLOOP_DELAY_MS)
            );

            continue;
        }

        const uint32_t raw_u32 =
            (raw >= 0) ? (uint32_t)raw : 0U;

        const uint32_t voltage_mv_u32 =
            (voltage_mv >= 0) ? (uint32_t)voltage_mv : 0U;

        const uint32_t filtered_mv =
            median_filter_update(
                &median_filter,
                voltage_mv_u32
            );

        if (!lut_initialized) {
            current_lut_index =
                light_lut_find_index(filtered_mv);

            target_servo_angle =
                light_servo_lut[current_lut_index]
                    .servo_angle_deg;

            lut_initialized = true;
        } else {
            const size_t new_lut_index =
                light_lut_update_index(
                    filtered_mv,
                    current_lut_index
                );

            if (new_lut_index != current_lut_index) {
                current_lut_index = new_lut_index;

                target_servo_angle =
                    light_servo_lut[current_lut_index]
                        .servo_angle_deg;
            }
        }

        servo_update_smooth();

        ESP_LOGI(
            TAG,
            "RAW=%lu, voltage=%lu mV, median=%lu mV, "
            "zone=%u, current=%lu deg, target=%lu deg",
            (unsigned long)raw_u32,
            (unsigned long)voltage_mv_u32,
            (unsigned long)filtered_mv,
            (unsigned int)current_lut_index,
            (unsigned long)current_servo_angle,
            (unsigned long)target_servo_angle
        );

        vTaskDelay(
            pdMS_TO_TICKS(SUPERLOOP_DELAY_MS)
        );
    }
}
