#include <Arduino.h>

const int LEDS[] = {3, 8, 18, 7, 16};
const int LED_COUNT = 5;

const int MIN_DELAY_TIME = 35;
const int ACCELERATION = 10;

void initRider() {
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
}

void flashLed(int index, int delayTime) {
  digitalWrite(LEDS[index], HIGH);
  delay(delayTime);
  digitalWrite(LEDS[index], LOW);
}

void moveForward(int delayTime){
    for (int i = 0; i < LED_COUNT; i++) {
    flashLed(i, delayTime);
  }
}

void moveBack(int delayTime){
    for (int i = LED_COUNT - 2; i > 0; i--) {
    flashLed(i, delayTime);
  }
}

void run(int delayTime) {
  moveForward(delayTime);
  moveBack(delayTime);
}

void patternKnightRider(){
    int delayTime = 220;
    while (delayTime > MIN_DELAY_TIME)
    {
        run(delayTime);
        delayTime -= ACCELERATION;
    }
}
