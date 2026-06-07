#include "app.h"
#include "main.h"

#include <stdint.h>

static volatile uint8_t active = 0;
static volatile uint8_t button_event = 0;
static volatile uint32_t last_button_time = 0;
static volatile uint32_t blink_speed = 1000;

void setup(void)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void loop(void)
{
    if (active)
    {
        if (blink_speed < 100)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            active = 0;
            button_event = 0;
            blink_speed = 1000;
        }
        else
        {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            HAL_Delay(blink_speed);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BTN_Pin)
    {
        uint32_t now = HAL_GetTick();

        if (now - last_button_time >= 50)
        {
            last_button_time = now;
            button_event++;
            blink_speed /= button_event;
            active = 1;
        }
    }
}
