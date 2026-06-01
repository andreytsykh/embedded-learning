#pragma once

typedef enum {
    RED,
    RED_YELLOW,
    GREEN,
    GREEN_BLINKING,
    YELLOW,
    YELLOW_BLINKING
} traffic_light_state_t;

void traffic_light_init(void);
void traffic_light_start(void);
void traffic_light_toggle_yellow_blink(void);
traffic_light_state_t traffic_light_get_state(void);