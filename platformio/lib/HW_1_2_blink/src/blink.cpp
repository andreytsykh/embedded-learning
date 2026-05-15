#include <Arduino.h>

#define RED_PIN 3
#define BLUE_PIN 8

void red(bool active) { digitalWrite(RED_PIN, active ? HIGH : LOW); }
void blue(bool active) { digitalWrite(BLUE_PIN, active ? HIGH : LOW); }
void off()
{
    red(false);
    blue(false);
}

void init()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    off();
}

void patternStrobe(int delayMs)
{
    red(true);
    blue(false);
    delay(delayMs);
    red(false);
    blue(true);
    delay(delayMs);
}
