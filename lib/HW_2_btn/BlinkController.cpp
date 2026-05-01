#include "BlinkController.h"

void blinkInit(BlinkController &ctrl, Led *leds, size_t count)
{
  ctrl.leds = leds;
  ctrl.count = count;

  ctrl.currentDelay = 500;
  ctrl.minDelay = 100;
  ctrl.maxDelay = 1000;
  ctrl.step = 100;
  ctrl.enabled = false;

  for (size_t i = 0; i < ctrl.count; i++)
  {
    pinMode(ctrl.leds[i].pin, OUTPUT);
    digitalWrite(ctrl.leds[i].pin, ctrl.leds[i].state);
  }
}

void blinkOff(BlinkController &ctrl) {
  for (size_t i = 0; i < ctrl.count; i++) {
    ctrl.leds[i].state = LOW;
    digitalWrite(ctrl.leds[i].pin, ctrl.leds[i].state);
  }
}

void blinkEnable(BlinkController &ctrl) {
  ctrl.enabled = true;
}

void blinkDisable(BlinkController &ctrl) {
  ctrl.enabled = false;
  blinkOff(ctrl);
}

void delayWithBtnCheck(int waitTime, Button &extBtn, Button &bootBtn)
{
  unsigned long start = millis();
  while (millis() - start < waitTime)
  {
    updateButtonState(extBtn);
    updateButtonState(bootBtn);
    delay(5);
  }
}

void blinkUpdate(BlinkController &ctrl, Button &extBtn, Button &bootBtn)
{
  if (!ctrl.enabled) {
    blinkOff(ctrl);
    // delayWithBtnCheck(20, extBtn, bootBtn);
    return;
  }

  for (size_t i = 0; i < ctrl.count; i++)
  {
    ctrl.leds[i].state = !ctrl.leds[i].state;
    digitalWrite(ctrl.leds[i].pin, ctrl.leds[i].state);

    delayWithBtnCheck(ctrl.currentDelay, extBtn, bootBtn);

    ctrl.leds[i].state = !ctrl.leds[i].state;
    digitalWrite(ctrl.leds[i].pin, ctrl.leds[i].state);
  }
}

void blinkFaster(BlinkController &ctrl)
{
  ctrl.currentDelay -= ctrl.step;

  if (ctrl.currentDelay < ctrl.minDelay)
  {
    ctrl.currentDelay = ctrl.minDelay;
  }
}

void blinkSlower(BlinkController &ctrl)
{
  ctrl.currentDelay += ctrl.step;

  if (ctrl.currentDelay > ctrl.maxDelay)
  {
    ctrl.currentDelay = ctrl.maxDelay;
  }
}