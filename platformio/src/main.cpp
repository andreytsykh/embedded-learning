#include <Arduino.h>

const uint16_t LDR_PIN = 4;
const uint16_t LED_PINS[4] = {5, 6, 7, 8};
const int16_t  SENSITIVITY = 0;
const uint16_t HYSTERESIS = 150;
const uint16_t BASE_THRESHOLDS[4] = {3000, 2300, 1600, 900};

const uint16_t WINDOW_SIZE = 16;
uint16_t samples[WINDOW_SIZE];
uint16_t sampleIndex = 0;
int32_t  sampleSum = 0;
bool bufferFilled = false;

uint16_t prevLedCount = 0;

uint16_t readMovingAverage() {
  sampleSum -= samples[sampleIndex];
  samples[sampleIndex] = analogRead(LDR_PIN);
  sampleSum += samples[sampleIndex];

  sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;
  if (sampleIndex == 0) bufferFilled = true;

  uint16_t count = bufferFilled ? WINDOW_SIZE : sampleIndex;
  uint16_t avg = sampleSum / count;
  uint16_t rounded = (avg / 10) * 10;
  uint16_t diff = avg - rounded;
  rounded = diff < 5 ? rounded : rounded + 10;

  return rounded;
}

uint16_t getLedCount(uint16_t light) {
  uint16_t count = 0;
  for (uint16_t i = 0; i < 4; i++) {
    uint16_t threshold = BASE_THRESHOLDS[i] + SENSITIVITY;
    bool alreadyOn = (prevLedCount > i);
    uint16_t effectiveThreshold = alreadyOn
      ? threshold + HYSTERESIS
      : threshold;
    if (light < effectiveThreshold) count = i + 1;
  }
  return count;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(LDR_PIN, ANALOG);
  memset(samples, 0, sizeof(samples));
  for (uint16_t i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
}

void loop() {
  uint16_t light = readMovingAverage();
  uint16_t ledCount = getLedCount(light);
  prevLedCount = ledCount;

  for (uint16_t i = 0; i < 4; i++) {
    digitalWrite(LED_PINS[i], i < ledCount ? HIGH : LOW);
  }

  Serial.printf("Light: %4d | LEDs ON: %d\n", light, ledCount);
  delay(50);
}