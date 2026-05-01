#pragma once
#include <Arduino.h>

struct Button
{
    int pin;
    bool activeHigh;

    int lastReading;
    int stableState;

    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    unsigned long lastClickTime;

    int clickCount;

    bool clickEvent;
    bool doubleClickEvent;
    bool longPressEvent;
};

void buttonBegin(Button &btn, int pin, bool activeHigh);
void updateButtonState(Button &btn);

bool buttonClicked(Button &btn);
bool buttonDoubleClicked(Button &btn);
bool buttonLongPressed(Button &btn);