#include "safe_callbacks.h"

#include "buzzer.h"
#include "display.h"
#include "melodies.h"
#include "safe.h"
#include "stm_link.h"

static void send_stm_command(stm_command_t command)
{
    esp_err_t err = stm_send_command(command);
    if (err != ESP_OK) {
        ESP_LOGE(
            "safe_callbacks",
            "STM command failed: %s",
            esp_err_to_name(err)
        );
    }
}

void safe_unlock_action(void){
    ESP_LOGI("safe_unlock_action","UNLOCKED");
    send_stm_command(STM_COMMAND_OPEN);
    safe_indication_off();
    safe_indication_on();
    safe_indication_activate_led(5);
    oled_display_show_message("UNLOCKED");
}

void safe_lock_action(void){
    ESP_LOGI("safe_lock_action","LOCKED");
    send_stm_command(STM_COMMAND_CLOSE);
    safe_indication_off();
    safe_indication_on();
    safe_indication_blink_led(4);
    oled_display_show_message("LOCKED");
}

void safe_success_sound(void){
    (void)buzzer_play_melody(&safe_unlocked_melody, false);
}

void safe_alarm_sound(void){
    (void)buzzer_play_melody(&safe_locked_melody, true);
}

void safe_confirm_digit_cb(uint8_t digit_index, uint8_t digit_value){
    ESP_LOGI("safe_confirm_digit_cb","Number confirmation");
     ESP_LOGI("safe_confirm_digit_cb","Index %u", digit_index);
    // safe_indication_activate_led(digit_index);

    // if (digit_index < 3U) {
    //     safe_indication_blink_led((uint8_t)(digit_index + 1U));
    // }
    
}

void safe_digit_changed_cb(uint8_t digit_index, uint8_t digit_value){
    ESP_LOGI("SF CALLBACK","Number changed");
    oled_display_show_code(
        digit_index,
        digit_value,
        safe_get_attempts_remaining()
    );
}
