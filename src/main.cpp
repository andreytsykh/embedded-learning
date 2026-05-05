#include <Arduino.h>
#include <math.h>

#define LDR_PIN 4
#define SAMPLES 50

const int RESOLUTIONS[] = {9, 10, 11, 12};

const adc_attenuation_t ATTENUATIONS[] = {
    ADC_0db,
    ADC_2_5db,
    ADC_6db,
    ADC_11db};

const char *ATTENUATION_NAMES[] = {
    "ADC_0db",
    "ADC_2_5db",
    "ADC_6db",
    "ADC_11db"};


const double ATTENUATION_MAX_V[] = {
    0.95, // ADC_0db
    1.25, // ADC_2_5db
    1.75, // ADC_6db
    3.10  // ADC_11db
};

void testCombination(int resolution_bits, int att_index)
{
  analogReadResolution(resolution_bits);
  analogSetPinAttenuation(LDR_PIN, ATTENUATIONS[att_index]);

  delay(300);

  const int max_raw = (1 << resolution_bits) - 1;

  uint16_t min_raw = max_raw;
  uint16_t max_raw_value = 0;

  double min_voltage = 999.0;
  double max_voltage = 0.0;

  double sum_voltage = 0.0;
  double sum_error = 0.0;

  for (int i = 0; i < SAMPLES; i++)
  {
    uint16_t raw = analogRead(LDR_PIN);
    double voltage_calc = raw * ATTENUATION_MAX_V[att_index] / max_raw;

    uint16_t voltage_mv = analogReadMilliVolts(LDR_PIN);
    double voltage_mv_v = voltage_mv / 1000.0;

    double error = 0.0;
    if (voltage_mv_v != 0.0)
    {
      error = fabs(voltage_calc - voltage_mv_v) / voltage_mv_v * 100.0;
    }

    if (raw < min_raw)
      min_raw = raw;
    if (raw > max_raw_value)
      max_raw_value = raw;

    if (voltage_mv_v < min_voltage)
      min_voltage = voltage_mv_v;
    if (voltage_mv_v > max_voltage)
      max_voltage = voltage_mv_v;

    sum_voltage += voltage_mv_v;
    sum_error += error;

    delay(50);
  }

  double avg_voltage = sum_voltage / SAMPLES;
  double avg_error = sum_error / SAMPLES;
  double stability = max_voltage - min_voltage;

  double step = ATTENUATION_MAX_V[att_index] / max_raw;

  Serial.println("--------------------------------------------------");
  Serial.printf("Resolution: %d bit | Attenuation: %s\n",
                resolution_bits, ATTENUATION_NAMES[att_index]);

  Serial.printf("RAW range: %u - %u\n", min_raw, max_raw_value);

  Serial.printf("Voltage range: %.3f V - %.3f V\n",
                min_voltage, max_voltage);

  Serial.printf("Average voltage: %.3f V\n", avg_voltage);
  Serial.printf("Stability (spread): %.4f V\n", stability);
  Serial.printf("Average error: %.2f %%\n", avg_error);

  Serial.printf("Resolution step: %.4f V\n", step);
}

void setup()
{
  Serial.begin(115200);
  uint16_t raw_init = analogRead(LDR_PIN);
  Serial.printf("Voltage:  %.3f V", raw_init * 3.3 / 4095.0);
}

void loop()
{
  static bool run_test = 1;
  if (run_test)
  {
    delay(1000);

    Serial.println("=== START ===");

    const int resolutions_count = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);
    const int attenuations_count = sizeof(ATTENUATIONS) / sizeof(ATTENUATIONS[0]);

    for (int r = 0; r < resolutions_count; r++)
    {
      for (int a = 0; a < attenuations_count; a++)
      {
        testCombination(RESOLUTIONS[r], a);
        delay(1000);
      }
    }

    Serial.println("=== DONE ===");
    run_test = 0;
  }
}