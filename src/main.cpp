#include <Arduino.h>
#include "Button.h"
#include "BlinkController.h"

#define EXT_BUTTON_PIN 4
#define BOOT_BUTTON_PIN 0

Button extBtn;
Button bootBtn;

Led leds[] = {
    {3, LOW},
    {8, LOW}};

BlinkController blinkCtrl;

void setup()
{
  Serial.begin(115200);

  buttonBegin(extBtn, EXT_BUTTON_PIN, true);    // HIGH = pressed
  buttonBegin(bootBtn, BOOT_BUTTON_PIN, false); // LOW = pressed

  blinkInit(blinkCtrl, leds, sizeof(leds) / sizeof(leds[0]));
}

void loop()
{
  updateButtonState(extBtn);
  updateButtonState(bootBtn);

  if (buttonClicked(extBtn))
  {
    Serial.print("Faster!");
    blinkFaster(blinkCtrl);
  }

  if (buttonClicked(bootBtn))
  {
    Serial.print("Slower!");
    blinkSlower(blinkCtrl);
  }
  blinkUpdate(blinkCtrl, extBtn, bootBtn);
}