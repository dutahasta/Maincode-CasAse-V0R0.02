#ifndef METER_VALUE_H
#define METER_VALUE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <PZEM6L24.h>
#include "config.h"

extern HardwareSerial pzemSerial;
extern PZEM6L24 pzem;
extern float acVoltage;
extern float acCurrent;
extern float acPower;
extern float acEnergy;

inline void initPzem()
{
  pzem.begin(9600);
  pzem.setEnable(PZEM_DE);
}

inline float readCurrentAverage()
{
  float total = 0.0;
  uint8_t valid = 0;

  // phase C = 2
  for (int i = 0; i < 10; i++)
  {
    float c = pzem.readCurrent(2);

    if (!isnan(c))
    {
      total += c;
      valid++;
    }

    delay(10);
  }

  if (valid == 0)
    return 0;

  float avg = total / valid;
  avg *= 0.82; // calibration

  if (avg < 0.02)
    avg = 0;

  return avg;
}

inline float readPowerAverage()
{
  float total = 0.0;
  uint8_t valid = 0;

  // phase C = 2, ambil 5 sample untuk power
  for (int i = 0; i < 5; i++)
  {
    float p = pzem.readActivePower(2);

    if (!isnan(p) && p >= 0)
    {
      total += p;
      valid++;
    }

    delay(20);
  }

  if (valid == 0)
    return acPower; // Return last valid value

  float avg = total / valid;
  return avg;
}

inline void updatePzemMeasurements()
{
  float v = pzem.readVoltage(2);
  if (!isnan(v) && v > 0)
    acVoltage = v;

  acCurrent = readCurrentAverage();

  // Use averaged power reading for stability
  float p = readPowerAverage();
  acPower = p;

  float e = pzem.readActiveEnergy(2);
  if (!isnan(e) && e >= 0)
    acEnergy = e;
}

#endif // METER_VALUE_H