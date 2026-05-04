#include <Arduino.h>

#define BUTTON_PIN 4

volatile uint32_t edgeCounter = 0;

void IRAM_ATTR buttonISR() {
  edgeCounter++;
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT); // зовнішній pull-down
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, RISING);
}

void loop() {
  static bool measuring = false;
  static uint32_t startCount = 0;
  static uint32_t lastSeenCount = 0;
  static uint32_t lastEdgeTime = 0;

  uint32_t count = edgeCounter;

  if (!measuring && count != lastSeenCount) {
    measuring = true;
    startCount = lastSeenCount;
    lastEdgeTime = millis();
  }

  if (measuring && count != lastSeenCount) {
    lastSeenCount = count;
    lastEdgeTime = millis();
  }

  if (measuring && digitalRead(BUTTON_PIN) == LOW && millis() - lastEdgeTime > 100) {
    Serial.printf("One full press RISING3 edges: %lu\n", lastSeenCount - startCount);
    measuring = false;
  }
}