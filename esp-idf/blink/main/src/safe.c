#include "safe.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_log.h"
#include "safe_indication.h"

#define SAFE_CODE_LENGTH   4U
#define SAFE_MAX_ATTEMPTS  3U

static const char *TAG = "safe";

static const uint8_t SECRET_CODE[SAFE_CODE_LENGTH] = { 4U, 2U, 7U, 1U };

typedef struct {
    safe_state_t            state;
    uint8_t                 code[SAFE_CODE_LENGTH];
    uint8_t                 digit_index;
    uint8_t                 current_digit;
    uint8_t                 attempts_used;
    safe_direction_t        current_direction;
    safe_direction_t        previous_direction;
    safe_digit_changed_cb_t on_digit_changed;
} safe_ctx_t;

static safe_ctx_t ctx;

__attribute__((weak)) void safe_unlock_action(void) {}
__attribute__((weak)) void safe_lock_action(void)   {}
__attribute__((weak)) void safe_success_sound(void) {}
__attribute__((weak)) void safe_alarm_sound(void)   {}
__attribute__((weak)) void safe_confirm_digit_cb(uint8_t digit_index, uint8_t digit_value) {}

static void reset_input(void)
{
    memset(ctx.code, 0, sizeof(ctx.code));
    ctx.digit_index   = 0U;
    ctx.current_digit = 0U;
    ctx.previous_direction = NONE;
    ctx.current_direction = NONE;
}

static void restart_input_indication(void)
{
    safe_indication_off();
    safe_indication_on();
    safe_indication_blink_led(ctx.digit_index);

    if (ctx.on_digit_changed) {
        ctx.on_digit_changed(ctx.digit_index, ctx.current_digit);
    }
}

static bool code_matches(void)
{
    return memcmp(ctx.code, SECRET_CODE, SAFE_CODE_LENGTH) == 0;
}

static void on_unlock(void)
{
    ctx.state = SAFE_STATE_OPEN;
    ESP_LOGI(TAG, "ACCESS GRANTED");
    safe_unlock_action();
    safe_success_sound();
}

static void on_failed_attempt(const char *reason)
{
    safe_indication_off();
    ctx.attempts_used++;
    ESP_LOGI(TAG, "Attempt failed: %s (%u/%u)",
             reason, ctx.attempts_used, SAFE_MAX_ATTEMPTS);

    if (ctx.attempts_used >= SAFE_MAX_ATTEMPTS) {
        ctx.state = SAFE_STATE_LOCKED;
        ESP_LOGE(TAG, "ACCESS BLOCKED");
        safe_lock_action();
        safe_alarm_sound();
        return;
    }

    reset_input();
    restart_input_indication();
}

static void handle_rotation(safe_direction_t direction){
    ctx.previous_direction = ctx.current_direction;
    ctx.current_direction = direction;

    if (ctx.current_direction == ctx.previous_direction || ctx.previous_direction == NONE)
    {
        ctx.current_digit = (uint8_t)((ctx.current_digit + 1U) % 10U);

        if (ctx.on_digit_changed) {
            ctx.on_digit_changed(ctx.digit_index, ctx.current_digit);
        }

        return;
    }

    if(ctx.current_direction != ctx.previous_direction){
        ctx.code[ctx.digit_index] = ctx.current_digit;
        safe_confirm_digit_cb(ctx.digit_index, ctx.current_digit);

        if(ctx.digit_index == SAFE_CODE_LENGTH-1){
            if (code_matches()) {
                on_unlock();
            } else {
                on_failed_attempt("incorrect PIN");
            }

            return;
        }

        ctx.digit_index++;
        ctx.current_digit = 0U;

        if (ctx.on_digit_changed) {
            ctx.on_digit_changed(ctx.digit_index, ctx.current_digit);
        }
    }   
}

void safe_init(const safe_config_t *config)
{
    ctx = (safe_ctx_t){
        .state          = SAFE_STATE_ENTERING,
        .on_digit_changed = config ? config->on_digit_changed : NULL,
    };
    reset_input();
    // safe_lock_action();
    ESP_LOGI(TAG, "Safe initialised — enter PIN");
    safe_indication_blink_led(ctx.digit_index);
}

void safe_process_event(safe_event_t event)
{
    if (ctx.state != SAFE_STATE_ENTERING) {
        return;
    }

    switch (event) {
        case SAFE_EVENT_ROTATE_CW:  handle_rotation(CW);  break;
        case SAFE_EVENT_ROTATE_CCW: handle_rotation(CCW); break;
        case SAFE_EVENT_RESET:      on_failed_attempt("manual reset"); break;
        default: break;
    }
}

safe_state_t safe_get_state(void)         { return ctx.state; }
uint8_t      safe_get_attempts_used(void) { return ctx.attempts_used; }
