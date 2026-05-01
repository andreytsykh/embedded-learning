#include "Button.h"

const unsigned long DEBOUNCE_DELAY = 20;
const unsigned long LONG_PRESS_DELAY = 700;
const unsigned long DOUBLE_CLICK_DELAY = 300;

void buttonBegin(Button &btn, int pin, bool activeHigh)
{
    btn.pin = pin;
    btn.activeHigh = activeHigh;

    pinMode(pin, activeHigh ? INPUT : INPUT_PULLUP);

    btn.lastReading = digitalRead(pin);
    btn.stableState = btn.lastReading;

    btn.lastDebounceTime = 0;
    btn.pressStartTime = 0;
    btn.lastClickTime = 0;

    btn.clickCount = 0;

    btn.clickEvent = false;
    btn.doubleClickEvent = false;
    btn.longPressEvent = false;
}

bool isPressed(Button &btn, int state)
{
    return btn.activeHigh ? state == HIGH : state == LOW;
}

void updateButtonState(Button &btn)
{
    int reading = digitalRead(btn.pin);

    if (reading != btn.lastReading)
    {
        btn.lastDebounceTime = millis();
    }

    if (millis() - btn.lastDebounceTime > DEBOUNCE_DELAY)
    {
        if (reading != btn.stableState)
        {
            btn.stableState = reading;
            if (isPressed(btn, btn.stableState))
            {
                btn.pressStartTime = millis();
                btn.longPressEvent = false;
            }
            else
            {
                btn.clickCount++;
                btn.lastClickTime = millis();
            }
        }
    }

    if (isPressed(btn, btn.stableState) && !btn.longPressEvent)
    {
        if (millis() - btn.pressStartTime > LONG_PRESS_DELAY)
        {
            btn.longPressEvent = true;
        }
    }

    if (btn.clickCount > 0 && millis() - btn.lastClickTime > DOUBLE_CLICK_DELAY)
    {
        if (btn.clickCount == 1)
        {
            btn.clickEvent = true;
        }
        else
        {
            btn.doubleClickEvent = true;
        }

        btn.clickCount = 0;
    }
    btn.lastReading = reading;
}

bool buttonClicked(Button &btn)
{
    if (btn.clickEvent)
    {
        btn.clickEvent = false;
        return true;
    }

    return false;
}

bool buttonDoubleClicked(Button &btn)
{
    if (btn.doubleClickEvent)
    {
        btn.doubleClickEvent = false;
        return true;
    }

    return false;
}

bool buttonLongPressed(Button &btn)
{
    return btn.longPressEvent;
}