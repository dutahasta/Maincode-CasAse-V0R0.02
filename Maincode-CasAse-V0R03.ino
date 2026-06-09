#include <Wire.h>
#include <string>
#include "ADS1X15.h"
#include "config.h"
#include "metervalue.h"
#include "hub.h"
#include "display.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#define PZEM_6L24
#include <PZEMPlus.h>

// ================= ADS1115 =================
#define ADS_CH    1

#define AVG_SAMPLES 16

/*
#define ZMPT_CH   2

const float ADS_LSB = 4.096 / 32768.0;

// Kalibrasi ZMPT
const float VOLTAGE_SCALE = 424.0;
const float OFFSET_VOLTAGE = 2.048;
*/

// ================= PZEM =================
HardwareSerial pzemSerial(2);

// phase C = index 2
PZEM6L24 pzem(pzemSerial, PZEM_RX, PZEM_TX, 0x01);

float acVoltage = 0;
float acCurrent = 0;
float acPower   = 0;
float acEnergy  = 0;

ADS1115 ADS(0x48);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ======================================
enum CPState
{
  STATE_A,
  STATE_B,
  STATE_C,
  STATE_D,
  STATE_EF,
  STATE_UNKNOWN
};

CPState currentState = STATE_UNKNOWN;

// ======================================
MeterValuePacket_t meterPacket;
DisplayPacket_t    displayPacket;
static uint8_t transactionCounter = 0;

// ======================================
unsigned long ledTimer = 0;
bool ledToggle = false;

// ======================================
int16_t readADS1115Stable(uint8_t ch)
{
  int32_t sum = 0;

  for (uint8_t i = 0; i < AVG_SAMPLES; i++)
  {
    sum += ADS.readADC(ch);
    delayMicroseconds(200);
  }

  int16_t raw = sum / AVG_SAMPLES;

  if (raw < 0)
    raw = 0;

  return raw;
}


// ======================================
float readCPVoltage()
{
  int16_t peak = 0;

  // Cari peak positif PWM
  for (int i = 0; i < 20; i++)
  {
    int16_t raw = ADS.readADC(ADS_CH);

    if (raw > peak)
      peak = raw;
  }

  // Convert raw -> volt
  float result = (((float)peak * 1.036) / 1128.0) - 11.702;

  return result;
}
/*
float readVoltageRMS()
{
  const int samples = 200;

  float sumSq = 0.0;

  for (int i = 0; i < samples; i++)
  {
    int16_t raw = ADS.readADC(ZMPT_CH);

    // ADC -> Volt
    float voltage = raw * ADS_LSB;

    // Hilangkan offset
    voltage -= OFFSET_VOLTAGE;

    // RMS accumulation
    sumSq += voltage * voltage;

    delayMicroseconds(200);
  }

  float rms = sqrt(sumSq / samples);

  // Scale ke tegangan PLN
  rms *= VOLTAGE_SCALE;

  return rms;
}
*/

// ======================================
CPState detectState(float cp)
{
  // =========================
  // STATE A
  // =========================
  if (cp >= 10.0)
    return STATE_A;

  // =========================
  // STATE B
  // =========================
  if (cp >= 7.0)
    return STATE_B;

  // =========================
  // STATE C
  // =========================
  if (cp >= 4.0)
    return STATE_C;

  // =========================
  // STATE D
  // =========================
  if (cp >= 2.0)
    return STATE_D;
  
  // =========================
  // ERROR
  // =========================
  return STATE_EF;
}

// ======================================

void setCharging(bool enable)
{
  digitalWrite(RELAY1, enable);
  digitalWrite(RELAY2, enable);
}

// ======================================

void updateLED(CPState state)
{
  switch(state)
  {
    // =========================================
    // STATE A
    // Semua LED OFF
    // =========================================
    case STATE_A:

      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);

      break;

    // =========================================
    // STATE B
    // LED1 ON
    // =========================================
    case STATE_B:

      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);

      break;

    // =========================================
    // STATE C
    // Blink bergantian
    // =========================================
    case STATE_C:

      if (millis() - ledTimer >= 90)
      {
        ledTimer = millis();

        ledToggle = !ledToggle;

        if (ledToggle)
        {
          digitalWrite(LED1, HIGH);
          digitalWrite(LED2, LOW);
        }
        else
        {
          digitalWrite(LED1, LOW);
          digitalWrite(LED2, HIGH);
        }
      }

      break;

    // =========================================
    // STATE D
    // Semua ON
    // =========================================
    case STATE_D:

      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);

      break;

    // =========================================
    // ERROR
    // Blink bersamaan
    // =========================================
    case STATE_EF:

      if (millis() - ledTimer >= 200)
      {
        ledTimer = millis();

        ledToggle = !ledToggle;

        digitalWrite(LED1, ledToggle);
        digitalWrite(LED2, ledToggle);
      }

      break;

    default:

      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);

      break;
  }
}

// ======================================
void printState(CPState state)
{
  switch(state)
  {
    case STATE_A:
      Serial.println("STATE A - EV NOT CONNECTED");
      break;

    case STATE_B:
      Serial.println("STATE B - EV CONNECTED");
      break;

    case STATE_C:
      Serial.println("STATE C - CHARGING");
      break;

    case STATE_D:
      Serial.println("STATE D - VENT REQUIRED");
      break;

    case STATE_EF:
      Serial.println("STATE E/F - ERROR");
      break;

    default:
      Serial.println("UNKNOWN");
      break;
  }
}

// ======================================
void buildAndSendHUB1Packet(CPState state, float tempC, float acVoltage, float acCurrent, float acPower, float acEnergy)
{
  hub1_setMeterValues(&meterPacket,
                      1,  // connector ID
                      (uint32_t)round(acEnergy * 1000.0),
                      (uint16_t)round(acVoltage * 10.0),
                      (uint16_t)round(acCurrent * 10.0),
                      (uint32_t)round(acPower),
                      (uint16_t)round(tempC * 10.0),
                      (state == STATE_A) ? HUB1_CHARGING_STATUS_IDLE :
                      (state == STATE_B) ? HUB1_CHARGING_STATUS_PREPARING :
                      (state == STATE_C) ? HUB1_CHARGING_STATUS_CHARGING :
                      HUB1_CHARGING_STATUS_FAULT,
                      &meterPacket.transactionId[0]);
  
  meterPacket.transactionId[0] = transactionCounter++;
  hub1_calculateCRC16(&meterPacket);
  
  // Debug print
  hub1_printMeterPacket(&meterPacket);
  
  // Send packet to RS485
  sendRS485Packet(&meterPacket);
}

// ======================================
void buildAndSendDisplayPacket(CPState state, float tempC, float acVoltage, float acCurrent, float acPower, float acEnergy,
                                const char *vehicleId, uint32_t saldo, const char *platNomor)
{
  // Sync transaction ID dengan HUB1 packet
  memcpy(displayPacket.transactionId, meterPacket.transactionId, sizeof(displayPacket.transactionId));

  disp_setMeterValues(&displayPacket,
                      1,
                      (uint32_t)round(acEnergy * 1000.0),
                      (uint16_t)round(acVoltage * 10.0),
                      (uint16_t)round(acCurrent * 10.0),
                      (uint32_t)round(acPower),
                      (uint16_t)round(tempC * 10.0),
                      (state == STATE_A) ? DISP_CHARGING_STATUS_IDLE      :
                      (state == STATE_B) ? DISP_CHARGING_STATUS_PREPARING :
                      (state == STATE_C) ? DISP_CHARGING_STATUS_CHARGING  :
                      DISP_CHARGING_STATUS_FAULT,
                      displayPacket.transactionId,
                      vehicleId,
                      saldo,
                      platNomor);

  disp_calculateCRC16(&displayPacket);
  disp_sendPacket(&displayPacket);
}

// ======================================
void sendRS485Packet(MeterValuePacket_t *packet)
{
  // Set DE pin HIGH to transmit
  digitalWrite(PZEM_DE, HIGH);
  delayMicroseconds(10);

  uint8_t buffer[HUB1_PACKET_SIZE];
  uint16_t len = 0;
  hub1_serializePacket(packet, buffer, &len);

  for (uint16_t i = 0; i < len; i++)
    pzemSerial.write(buffer[i]);

  // Wait for transmission to complete
  pzemSerial.flush();
  delayMicroseconds(100);

  // Set DE pin LOW to receive
  digitalWrite(PZEM_DE, LOW);
}

// ======================================
void setup()
{
  Serial.begin(115200);

  // PWM 
  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWM_PIN, PWM_CH);

  // Relay
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT); 

  // LED
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  setCharging(false);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // I2C
  Wire.begin();

  // UART1 display
  Serial1.begin(DISPLAY_UART_BAUD, SERIAL_8N1, DISPLAY_UART_RX, DISPLAY_UART_TX);
  Serial1.println("DISPLAY UART1 READY");

  // ADS1115
  ADS.begin();
  ADS.setGain(1);
  ADS.setDataRate(7);
  ADS.setMode(0);

  ADS.readADC(ADS_CH);

  // DS18B20
  sensors.begin();
  // =========================
  // PZEM
  // =========================
  initPzem();

  float test = pzem.readVoltage(2);

  if (!isnan(test))
  {
    Serial.println("PZEM CONNECTED");
  }
  else
  {
    Serial.println("PZEM NOT DETECTED");
  }

  // =========================
  // HUB1 Meter Packet Init
  // =========================
  hub1_initMeterPacket(&meterPacket);
  hub1_setIdentification(&meterPacket,
                         "00012345",     // Charger Box Serial
                         "AC-007KW",     // Charger Point Model
                         "00012345",     // Charger Point Serial
                         "QIMS-123",     // Charger Point Vendor
                         "V105.1.25.12"  // Firmware Version
  );

  // =========================
  // Display Packet Init
  // =========================
  disp_initPacket(&displayPacket);
  disp_setIdentification(&displayPacket,
                         "00012345",     // Charger Box Serial
                         "AC-007KW",     // Charger Point Model
                         "00012345",     // Charger Point Serial
                         "QIMS-123",     // Charger Point Vendor
                         "V105.1.25.12"  // Firmware Version
  );

  // PWM sementara
  ledcWrite(PWM_CH, 255);
  Serial.println("IEC61851 AC Charger Started");
}

// ======================================

void loop()
{
  // =========================
  // READ CP
  // =========================
  float cpVoltage = readCPVoltage();
  CPState newState = detectState(cpVoltage);
  // =========================
  // READ TEMPERATURE
  // =========================
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  //float acVoltage = readVoltageRMS();
  // =========================
  // READ PZEM PHASE C
  // =========================
  updatePzemMeasurements();

  // =========================
  // SERIAL MONITOR
  // =========================
  Serial.print("Temperature: ");
  Serial.print(tempC,1);
  Serial.print(" C | "); 
  
  Serial.print("AC: ");
  Serial.print(acVoltage,1);
  Serial.print(" V | "); 
  
  Serial.print(acCurrent,2);
  Serial.print(" A | "); 
  
  Serial.print(acPower / 1000.0, 1);
  Serial.print(" kW | ");
  
  Serial.print(acEnergy,1);
  Serial.print(" kWh | ");

  Serial.print("CP Voltage: ");
  Serial.print(cpVoltage, 3);
  Serial.print(" V | ");
  
  printState(newState);
  
  // Build and send HUB1 meter packet
  buildAndSendHUB1Packet(newState, tempC, acVoltage, acCurrent, acPower, acEnergy);
  
  // vehicleId, saldo, platNomor diisi dari data transaksi aktif
  buildAndSendDisplayPacket(newState, tempC, acVoltage, acCurrent, acPower, acEnergy,
                             "", 0, "");

  // ==================================
  // IEC61851 Logic
  // ==================================

switch(newState)
{
  // ==================================
  // STATE A
  // ==================================
  case STATE_A:

    // +12V flat
    ledcWrite(PWM_CH, 255);

    setCharging(false);

    break;

  // ==================================
  // STATE B
  // ==================================
  case STATE_B:

    // Advertise current
    setCPDuty(32);

    setCharging(false);

  break;
  // ==================================
  // STATE C
  // ==================================
  case STATE_C:

    // PWM tetap aktif
    setCPDuty(32);
    // Relay ON
    setCharging(true);

  break;

  // ==================================
  // STATE D
  // ==================================
  case STATE_D:

    setCPDuty(16);
    setCharging(false);

  break;

  // ==================================
  // ERROR
  // ==================================
  case STATE_EF:

    // PWM OFF
    ledcWrite(PWM_CH, 0);

    setCharging(false);

  break;

  default:

    ledcWrite(PWM_CH, 0);

    setCharging(false);

  break;
}

  // LED Update
  updateLED(newState);

  delay(10);
}

std::string snake_case_to_camel_case(const std::string& str){
  std::string result;
  bool toUpper = false;

  for(char ch : str){

    if (ch == '_'){
      toUpper = true;

    } else if (ch < '0' || (ch > '9' && ch < 'A') || (ch > 'Z' && ch < 'a') || ch > 'z'){
      toUpper = true;

    } else {

      if(toUpper){
        result += toupper(ch);
        toUpper = false;

      } else {
        result += ch;
      }
    }
  }

  return result;
}

void setCPDuty(float ampere)
{
  float duty = ampere / 0.6;

  if (duty > 100.0)
      duty = 100.0;

  uint8_t pwm = (255.0 * duty) / 100.0;

  ledcWrite(PWM_CH, pwm);
}