#ifndef METER_VALUE_H
#define METER_VALUE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#define PZEM_6L24
#include <PZEMPlus.h>
#include "config.h"

extern HardwareSerial pzemSerial;
extern PZEM6L24 pzem;
extern float acVoltage;
extern float acCurrent;
extern float acPower;
extern float acEnergy;

// Phase C pada PZEM 3 fasa = index 2 (R=0, S=1, C=2)
#define PZEM_PHASE_C    2
#define PZEM_READ_DELAY 100

inline void initPzem()
{
  pzem.begin(9600);
  pzem.setEnable(PZEM_DE);
}

// ============================================================
// READ PZEM PHASE C (port dari test 3-fasa, single phase)
//  - NaN / nilai tidak valid  -> pertahankan nilai terakhir
//  - power dibuat absolut, tolak data corrupt (> V*I*1.5)
//  - noise power < 3W         -> 0
//  - energy tidak boleh turun
// ============================================================
inline void updatePzemMeasurements()
{
  static float lastVoltage = 0;
  static float lastCurrent = 0;
  static float lastPower   = 0;
  static float lastEnergy  = 0;

  // VOLTAGE
  float v = pzem.readVoltage(PZEM_PHASE_C);
  delay(PZEM_READ_DELAY);

  // CURRENT
  float c = pzem.readCurrent(PZEM_PHASE_C);
  delay(PZEM_READ_DELAY);

  // POWER
  float p = pzem.readActivePower(PZEM_PHASE_C);
  delay(PZEM_READ_DELAY);

  // ENERGY
  float e = pzem.readActiveEnergy(PZEM_PHASE_C);
  delay(PZEM_READ_DELAY);

  // =========================
  // FILTER VOLTAGE
  // =========================
  if (!isnan(v) && v > 0)
  {
    lastVoltage = v;
  }

  // =========================
  // FILTER CURRENT
  // =========================
  if (!isnan(c) && c >= 0)
  {
    lastCurrent = c;
  }

  // =========================
  // FILTER POWER
  // =========================
  if (!isnan(p))
  {
    p = fabsf(p);

    float expectedPower = lastVoltage * lastCurrent;

    // reject data corrupt
    if (p <= (expectedPower * 1.5f))
    {
      lastPower = p;
    }

    // noise kecil
    if (lastPower < 3.0f)
    {
      lastPower = 0;
    }
  }

  // =========================
  // FILTER ENERGY
  // ========================= 
  if (!isnan(e))
  {
    if (e >= lastEnergy)
    {
      lastEnergy = e;
    }
  }

  // SAVE FINAL
  acVoltage = lastVoltage;
  acCurrent = lastCurrent;
  acPower   = lastPower;
  acEnergy  = lastEnergy;
}

#endif // METER_VALUE_H
