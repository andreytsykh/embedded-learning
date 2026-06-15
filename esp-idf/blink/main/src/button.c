#include "button.h"
#include "esp_check.h"

static const char *TAG = "button";

static void debounce_cb(void *arg);
static void long_press_cb(void *arg);

static void IRAM_ATTR button_isr_handler(void *arg)
{
    button_t *btn = (button_t *)arg;

    gpio_intr_disable(btn->gpio);
    esp_timer_start_once(btn->debounce_timer, btn->debounce_us);
}

static void debounce_cb(void *arg)
{
    button_t *btn = (button_t *)arg;
    int level = gpio_get_level(btn->gpio);

    if (level == 1 && btn->state == BUTTON_IDLE)
    {
        btn->state = BUTTON_PRESSING;
        esp_timer_start_once(btn->long_press_timer, btn->long_press_us);
    }
    else if (level == 0 && btn->state == BUTTON_PRESSING)
    {
        btn->state = BUTTON_SHORT_PRESS;
        esp_timer_stop(btn->long_press_timer);

        if (btn->on_short_press)
        {
            btn->on_short_press(btn->ctx);
        }

        btn->state = BUTTON_IDLE;
    }
    else if (level == 0 && btn->state == BUTTON_LONG_PRESS)
    {
        btn->state = BUTTON_IDLE;
    }
    gpio_intr_enable(btn->gpio);
}

static void long_press_cb(void *arg)
{
    button_t *btn = (button_t *)arg;

    if (gpio_get_level(btn->gpio) == 1 && btn->state == BUTTON_PRESSING)
    {
        btn->state = BUTTON_LONG_PRESS;

        if (btn->on_long_press)
        {
            btn->on_long_press(btn->ctx);
        }
    }
}

void button_init(button_t *btn)
{
    gpio_config_t button_config = {
        .pin_bit_mask = 1ULL << btn->gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE};

    ESP_ERROR_CHECK(gpio_config(&button_config));

    esp_timer_create_args_t debounce_args = {
        .callback = debounce_cb,
        .arg = btn,
        .name = "button_debounce"};

    ESP_ERROR_CHECK(esp_timer_create(&debounce_args, &btn->debounce_timer));

    esp_timer_create_args_t long_press_args = {
        .callback = long_press_cb,
        .arg = btn,
        .name = "button_long_press"};

    ESP_ERROR_CHECK(esp_timer_create(&long_press_args, &btn->long_press_timer));;
    ESP_ERROR_CHECK(gpio_isr_handler_add(btn->gpio, button_isr_handler, btn));

    int level = gpio_get_level(btn->gpio);

    if (level == 1)
    {
        btn->state = BUTTON_PRESSING;
        esp_timer_start_once(btn->long_press_timer, btn->long_press_us);
    }
    else
    {
        btn->state = BUTTON_IDLE;
    }
}
