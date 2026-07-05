#include "safe_callbacks.h"

#include "buzzer.h"
#include "melodies.h"

void safe_unlock_action(void){
    ESP_LOGI("safe_lock_action","UNLOCKED");
    safe_indication_off();
    safe_indication_on();
    safe_indication_activate_led(5);
}

void safe_lock_action(void){
    ESP_LOGI("safe_lock_action","LOCKED");
    safe_indication_off();
    safe_indication_on();
    safe_indication_blink_led(4);
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
    safe_indication_activate_led(digit_index);

    if (digit_index < 3U) {
        safe_indication_blink_led((uint8_t)(digit_index + 1U));
    }
    
}

void safe_digit_changed_cb(uint8_t digit_index, uint8_t digit_value){
    ESP_LOGI("SF CALLBACK","Number changed");
    safe_indication_show_digit(digit_value);
}
