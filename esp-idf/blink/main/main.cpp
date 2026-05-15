#include <cstdint>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "BLINK";
 
 
enum class LedState : uint8_t
{
    OFF = 0,
    ON  = 1
};
 
 
class Config
{
public:
    static constexpr uint8_t   LED_PIN               = 4;
 
    static constexpr uint32_t  BLINK_PERIOD_MS        = 500U;  // період блимання
    static constexpr uint32_t  LOOP_TICK_MS            = 10U;   // крок superloop
 
private:
    Config() = delete;
};
 
class Led
{
public:
    Led(uint8_t pin = Config::LED_PIN) noexcept
        : m_pin(pin)
        , m_state(LedState::OFF)
    {}
 
    void init() noexcept
    {
        gpio_set_direction(static_cast<gpio_num_t>(m_pin),GPIO_MODE_OUTPUT);
        ESP_LOGI(TAG, "LED init on GPIO %d", m_pin);
        set(LedState::OFF);
    }
 
    void set(LedState state) noexcept
    {
        m_state = state;
        ESP_LOGI(TAG, "LED -> %s", (state == LedState::ON) ? "ON" : "OFF");
        gpio_set_level(static_cast<gpio_num_t>(m_pin),
                       static_cast<uint32_t>(state));
    }
 
    LedState state() const noexcept
    {
        return m_state;
    }
 
private:
    const uint8_t     m_pin;
    volatile LedState m_state;
};
 
 
static volatile LedState g_led_state = LedState::OFF;
 
extern "C" void app_main(void)
{
    Led led;
    led.init();
 
    int64_t last_toggle_us = esp_timer_get_time();
 
    while (true)
    {
        const int64_t  now_us     = esp_timer_get_time();
        const uint32_t elapsed_ms = static_cast<uint32_t>((now_us - last_toggle_us) / 1000LL);
 
        if (elapsed_ms >= Config::BLINK_PERIOD_MS)
        {
            last_toggle_us = now_us;
 
            g_led_state = (g_led_state == LedState::OFF)
                              ? LedState::ON
                              : LedState::OFF;
 
            led.set(g_led_state);
        }
 
        vTaskDelay(pdMS_TO_TICKS(Config::LOOP_TICK_MS));
    }
}