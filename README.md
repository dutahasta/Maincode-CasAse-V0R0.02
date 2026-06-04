# AC Charging Master Controller - ESP32

## Deskripsi
Sistem pengontrol AC Charging untuk EV (Electric Vehicle) berbasis ESP32. Mengimplementasikan standar IEC 61851-1 dengan fitur monitoring real-time dan komunikasi RS485 ke sistem HUB.

## Fitur Utama
- ✅ **IEC 61851 Control Pilot (CP)** - Deteksi state kendaraan (A/B/C/D/E/F)
- ✅ **PZEM Power Meter** - Monitoring tegangan, arus, daya, energi 3-phase
- ✅ **DS18B20 Temperature Sensor** - Monitoring suhu charging connector
- ✅ **PWM Control** - Duty cycle control untuk advertise arus charging
- ✅ **RS485 Communication** - Kirim data meter ke HUB setiap cycle
- ✅ **Relay Control** - Kontrol saklar charging via relay
- ✅ **LED Indicator** - Status visual berdasarkan CP state
- ✅ **UART Display** - Serial output untuk monitoring

## Hardware Components
| Komponen | Pin | Fungsi |
|----------|-----|--------|
| **PWM (CP)** | GPIO 12 | Control Pilot PWM signal |
| **ADS1115** | GPIO 21, 22 (I2C) | ADC untuk baca CP voltage |
| **PZEM-6L-24** | GPIO 16, 17, 13 (UART2) | 3-phase power meter |
| **DS18B20** | GPIO 15 | Temperature sensor (OneWire) |
| **Relay 1,2** | GPIO 25, 26 | Kontrol charging relay |
| **LED 1,2** | GPIO 32, 33 | Status indicator |
| **Display UART** | GPIO 14, 27 (UART1) | Serial display output |

## CP State Detection
```
STATE A:  CP >= 10V      → EV tidak terhubung
STATE B:  7V <= CP < 10V → EV terhubung (standby)
STATE C:  4V <= CP < 7V  → Charging aktif
STATE D:  2V <= CP < 4V  → Vent required
STATE E/F: CP < 2V       → Error/Diode check fail
```

## LED Indication
| State | LED | Indikasi |
|-------|-----|----------|
| A | OFF | Tidak ada EV |
| B | Menyala | EV terhubung |
| C | Blink alternating | Charging |
| D | Semua ON | Vent required |
| E/F | Blink together | Error |

## Software Architecture

### File Structure
```
Maincode-CasAse-V0R0.02/
├── Maincode-CasAse-V0R0.02.ino  (Main logic)
├── config.h                      (Pin & config)
├── hub.h                         (RS485 packet structure)
├── metervalue.h                  (PZEM functions)
└── README.md                     (This file)
```

### Main Functions
- `setup()` - Inisialisasi semua hardware
- `loop()` - Main cycle: baca sensor → detect state → kontrol → transmit
- `buildAndSendHUB1Packet()` - Build & kirim paket RS485
- `sendRS485Packet()` - Low-level RS485 transmit
- `updateLED()` - Update LED berdasarkan state
- `setCPDuty()` - Set PWM duty cycle

### Data Flow
```
loop() {
  1. Baca CP voltage (ADS1115)
  2. Detect state (STATE A/B/C/D/E/F)
  3. Baca temperature (DS18B20)
  4. Baca PZEM measurements (Voltage, Current, Power, Energy)
  5. Build meter packet
  6. Transmit RS485 ke HUB
  7. Update LED
  8. IEC61851 logic (PWM, Relay)
  9. Send display data (UART1)
  10. Delay 10ms
}
```

## RS485 Communication
- Sharing dengan PZEM via UART2
- Baud rate: 9600 bps
- Paket: 31 bytes dengan CRC16 Modbus
- Konten: Energy, Voltage, Current, Power, Temperature, Charging Status

Lihat `hub.h` untuk format paket detail.

## Configuration

### Edit pin di `config.h`
```c
#define PWM_PIN 12              // GPIO pin untuk PWM
#define PZEM_RX 16, PZEM_TX 17, PZEM_DE 13  // UART2 pins
#define RELAY1 25, RELAY2 26    // Relay pins
#define LED1 32, LED2 33        // LED pins
```

## Kompilasi & Upload
1. Arduino IDE → Board: **ESP32 Dev Module**
2. Perlu library:
   - `ADS1X15` (ADC)
   - `DallasTemperature` (Temperature)
   - `PZEM6L24` (Power meter)
   - `OneWire` (1-wire protocol)
3. Compile → Upload via Serial

## Serial Monitor Output
```
Temperature: 29.8 C | AC: 228.7 V | 0.03 A | 0.5 W | 0.600 kWh | CP Voltage: 6.211 V | STATE C - CHARGING
```

Hex packet RS485:
```
5A A5 1B 00 0F 04 01 01 17 00 00 00 00 00 58 02 00 00 F1 08 00 00 01 00 00 00 2B 01 02 74 12
```

## Troubleshooting

| Issue | Solusi |
|-------|--------|
| PZEM NOT DETECTED | Cek koneksi GPIO 16,17,13 dan baud rate 9600 |
| CP voltage invalid | Cek ADS1115 koneksi I2C |
| Relay tidak ON | Cek GPIO 25,26 dan power supply relay |
| Temperature invalid | Cek DS18B20 koneksi GPIO 15 |
| RS485 packet tidak keluar | Cek GPIO 17 TX dan pin DE 13 |

## Versi & Status
- **Versi:** V0R0.02
- **Status:** Development
- **Last Update:** June 4, 2026

---

**Dikembangkan untuk:** AC EV Charging Station  
**Target Hardware:** ESP32 Dev Module  
**Protocol:** Modbus RTU (RS485) + IEC 61851
