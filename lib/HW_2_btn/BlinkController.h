#pragma once
#include <Arduino.h>
#include "Button.h"

struct Led
{
    int pin;
    bool state;
};

struct BlinkController {
  Led* leds;
  size_t count;

  int currentDelay;
  int minDelay;
  int maxDelay;
  int step;

  bool enabled;
};

void blinkInit(BlinkController &ctrl, Led *leds, size_t count);
void blinkUpdate(BlinkController &ctrl, Button &extBtn, Button &bootBtn);

void blinkFaster(BlinkController &ctrl);
void blinkSlower(BlinkController &ctrl);

void blinkEnable(BlinkController &ctrl);
void blinkDisable(BlinkController &ctrl);
void blinkOff(BlinkController &ctrl);