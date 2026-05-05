#include <Arduino.h>

#define LDR_PIN 4

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint16_t  raw = analogRead(LDR_PIN);
  double voltage_calc = raw * 3.3 / 4095.0;
  uint16_t  voltage_mv = analogReadMilliVolts(LDR_PIN);
  double voltage_mv_v = voltage_mv / 1000.0;
  double error = fabs(voltage_calc - voltage_mv_v) / voltage_mv_v * 100;

  Serial.printf("RAW:  %u | Calculated Voltage: %.3f V | analogReadMilliVolts: %.3f V | Error(%): %.2f %%\n",
    raw, voltage_calc, voltage_mv_v, error);

  delay(100);
}