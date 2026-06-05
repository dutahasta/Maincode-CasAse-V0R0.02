#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <string.h>

// ===== UART1 Display Communication Constants =====
#define DISP_HEADER_BYTE1           0x7A
#define DISP_HEADER_BYTE2           0xA3
#define DISP_MESSAGE_TYPE_METER     0x0F

#define DISP_SOURCE_ID              0x04  // MASTER
#define DISP_DESTINATION_ID         0x02  // DISPLAY

// ===== Field Sizes =====
#define DISP_CHARGER_BOX_SERIAL_SIZE    8
#define DISP_CHARGER_POINT_MODEL_SIZE   8
#define DISP_CHARGER_POINT_SERIAL_SIZE  8
#define DISP_CHARGER_POINT_VENDOR_SIZE  8
#define DISP_FIRMWARE_VERSION_SIZE      16
#define DISP_VEHICLE_ID_SIZE            16
#define DISP_PLAT_NOMOR_SIZE            14

// ===== Packet Structure Offsets =====
#define DISP_OFFSET_HEADER                  0   // Bytes 0-1   (2 bytes)
#define DISP_OFFSET_LENGTH                  2   // Bytes 2-3   (2 bytes)
#define DISP_OFFSET_MESSAGE_TYPE            4   // Byte 4
#define DISP_OFFSET_SOURCE_ID               5   // Byte 5
#define DISP_OFFSET_DESTINATION_ID          6   // Byte 6
#define DISP_OFFSET_CHARGER_BOX_SERIAL      7   // Bytes 7-14  (8 bytes)
#define DISP_OFFSET_CHARGER_POINT_MODEL     15  // Bytes 15-22 (8 bytes)
#define DISP_OFFSET_CHARGER_POINT_SERIAL    23  // Bytes 23-30 (8 bytes)
#define DISP_OFFSET_CHARGER_POINT_VENDOR    31  // Bytes 31-38 (8 bytes)
#define DISP_OFFSET_FIRMWARE_VERSION        39  // Bytes 39-54 (16 bytes)
#define DISP_OFFSET_CONNECTOR_ID            55  // Byte 55
#define DISP_OFFSET_TRANSACTION_ID          56  // Bytes 56-61 (6 bytes)
#define DISP_OFFSET_ENERGY                  62  // Bytes 62-65 (4 bytes) + Byte 66 reserved
#define DISP_OFFSET_VOLTAGE                 67  // Bytes 67-68 (2 bytes)
#define DISP_OFFSET_CURRENT                 69  // Bytes 69-70 (2 bytes)
#define DISP_OFFSET_POWER                   71  // Bytes 71-74 (4 bytes)
#define DISP_OFFSET_TEMPERATURE             75  // Bytes 75-76 (2 bytes)
#define DISP_OFFSET_CHARGING_STATUS         77  // Byte 77
#define DISP_OFFSET_VEHICLE_ID              78  // Bytes 78-93 (16 bytes)
#define DISP_OFFSET_SALDO                   94  // Bytes 94-97 (4 bytes) + Byte 98 reserved
#define DISP_OFFSET_PLAT_NOMOR              99  // Bytes 99-112 (14 bytes)
#define DISP_OFFSET_CRC16                   113 // Bytes 113-114 (2 bytes)

#define DISP_PACKET_SIZE                    115 // Total packet size

// ===== Charging Status Codes =====
#define DISP_CHARGING_STATUS_IDLE       0x00
#define DISP_CHARGING_STATUS_PREPARING  0x01
#define DISP_CHARGING_STATUS_CHARGING   0x02
#define DISP_CHARGING_STATUS_FAULT      0x04

// ===== Display Packet Structure =====
typedef struct {
    uint8_t  header[2];                                                    // Bytes 0-1
    uint16_t length;                                                       // Bytes 2-3
    uint8_t  messageType;                                                  // Byte 4
    uint8_t  sourceId;                                                     // Byte 5
    uint8_t  destinationId;                                                // Byte 6
    char     chargerBoxSerial[DISP_CHARGER_BOX_SERIAL_SIZE];              // Bytes 7-14
    char     chargerPointModel[DISP_CHARGER_POINT_MODEL_SIZE];            // Bytes 15-22
    char     chargerPointSerial[DISP_CHARGER_POINT_SERIAL_SIZE];          // Bytes 23-30
    char     chargerPointVendor[DISP_CHARGER_POINT_VENDOR_SIZE];          // Bytes 31-38
    char     firmwareVersion[DISP_FIRMWARE_VERSION_SIZE];                 // Bytes 39-54
    uint8_t  connectorId;                                                  // Byte 55
    uint8_t  transactionId[6];                                             // Bytes 56-61
    uint32_t energy;                                                       // Bytes 62-65 (Wh)
    uint8_t  _energyRsvd;                                                  // Byte 66 (reserved = 0)
    uint16_t voltage;                                                      // Bytes 67-68 (V × 10)
    uint16_t current;                                                      // Bytes 69-70 (A × 10)
    uint32_t power;                                                        // Bytes 71-74 (W)
    uint16_t temperature;                                                  // Bytes 75-76 (°C × 10)
    uint8_t  chargingStatus;                                               // Byte 77
    char     vehicleId[DISP_VEHICLE_ID_SIZE];                             // Bytes 78-93
    uint32_t saldo;                                                        // Bytes 94-97 (Rupiah)
    uint8_t  _saldoRsvd;                                                   // Byte 98 (reserved = 0)
    char     platNomor[DISP_PLAT_NOMOR_SIZE];                             // Bytes 99-112
    uint16_t crc16;                                                        // Bytes 113-114
} DisplayPacket_t;

// Copy src string into fixed-width dest field, space-padded, no null terminator
static inline void disp_copyField(char *dest, const char *src, uint8_t size)
{
    memset(dest, ' ', size);
    if (src) {
        uint8_t len = 0;
        while (src[len] && len < size) len++;
        memcpy(dest, src, len);
    }
}

// Serialize packet to raw byte buffer; sets *outLen to DISP_PACKET_SIZE
static inline void disp_serializePacket(const DisplayPacket_t *packet, uint8_t *buffer, uint16_t *outLen)
{
    uint16_t i = 0;
    buffer[i++] = packet->header[0];
    buffer[i++] = packet->header[1];
    buffer[i++] = (uint8_t)(packet->length & 0xFF);
    buffer[i++] = (uint8_t)((packet->length >> 8) & 0xFF);
    buffer[i++] = packet->messageType;
    buffer[i++] = packet->sourceId;
    buffer[i++] = packet->destinationId;
    memcpy(&buffer[i], packet->chargerBoxSerial,   DISP_CHARGER_BOX_SERIAL_SIZE);   i += DISP_CHARGER_BOX_SERIAL_SIZE;
    memcpy(&buffer[i], packet->chargerPointModel,  DISP_CHARGER_POINT_MODEL_SIZE);  i += DISP_CHARGER_POINT_MODEL_SIZE;
    memcpy(&buffer[i], packet->chargerPointSerial, DISP_CHARGER_POINT_SERIAL_SIZE); i += DISP_CHARGER_POINT_SERIAL_SIZE;
    memcpy(&buffer[i], packet->chargerPointVendor, DISP_CHARGER_POINT_VENDOR_SIZE); i += DISP_CHARGER_POINT_VENDOR_SIZE;
    memcpy(&buffer[i], packet->firmwareVersion,    DISP_FIRMWARE_VERSION_SIZE);     i += DISP_FIRMWARE_VERSION_SIZE;
    buffer[i++] = packet->connectorId;
    memcpy(&buffer[i], packet->transactionId, 6); i += 6;
    buffer[i++] = (uint8_t)(packet->energy & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 8) & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 16) & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 24) & 0xFF);
    buffer[i++] = packet->_energyRsvd;
    buffer[i++] = (uint8_t)(packet->voltage & 0xFF);
    buffer[i++] = (uint8_t)((packet->voltage >> 8) & 0xFF);
    buffer[i++] = (uint8_t)(packet->current & 0xFF);
    buffer[i++] = (uint8_t)((packet->current >> 8) & 0xFF);
    buffer[i++] = (uint8_t)(packet->power & 0xFF);
    buffer[i++] = (uint8_t)((packet->power >> 8) & 0xFF);
    buffer[i++] = (uint8_t)((packet->power >> 16) & 0xFF);
    buffer[i++] = (uint8_t)((packet->power >> 24) & 0xFF);
    buffer[i++] = (uint8_t)(packet->temperature & 0xFF);
    buffer[i++] = (uint8_t)((packet->temperature >> 8) & 0xFF);
    buffer[i++] = packet->chargingStatus;
    memcpy(&buffer[i], packet->vehicleId, DISP_VEHICLE_ID_SIZE); i += DISP_VEHICLE_ID_SIZE;
    buffer[i++] = (uint8_t)(packet->saldo & 0xFF);
    buffer[i++] = (uint8_t)((packet->saldo >> 8) & 0xFF);
    buffer[i++] = (uint8_t)((packet->saldo >> 16) & 0xFF);
    buffer[i++] = (uint8_t)((packet->saldo >> 24) & 0xFF);
    buffer[i++] = packet->_saldoRsvd;
    memcpy(&buffer[i], packet->platNomor, DISP_PLAT_NOMOR_SIZE); i += DISP_PLAT_NOMOR_SIZE;
    buffer[i++] = (uint8_t)(packet->crc16 & 0xFF);
    buffer[i++] = (uint8_t)((packet->crc16 >> 8) & 0xFF);
    if (outLen) *outLen = i;
}

/**
 * Initialize display packet with default header and IDs
 */
static inline void disp_initPacket(DisplayPacket_t *packet)
{
    packet->header[0]      = DISP_HEADER_BYTE1;
    packet->header[1]      = DISP_HEADER_BYTE2;
    packet->length         = DISP_PACKET_SIZE - 4;
    packet->messageType    = DISP_MESSAGE_TYPE_METER;
    packet->sourceId       = DISP_SOURCE_ID;
    packet->destinationId  = DISP_DESTINATION_ID;
    memset(packet->chargerBoxSerial,   ' ', DISP_CHARGER_BOX_SERIAL_SIZE);
    memset(packet->chargerPointModel,  ' ', DISP_CHARGER_POINT_MODEL_SIZE);
    memset(packet->chargerPointSerial, ' ', DISP_CHARGER_POINT_SERIAL_SIZE);
    memset(packet->chargerPointVendor, ' ', DISP_CHARGER_POINT_VENDOR_SIZE);
    memset(packet->firmwareVersion,    ' ', DISP_FIRMWARE_VERSION_SIZE);
    packet->connectorId    = 1;
    memset(packet->transactionId, 0, sizeof(packet->transactionId));
    packet->energy         = 0;
    packet->_energyRsvd    = 0;
    packet->voltage        = 0;
    packet->current        = 0;
    packet->power          = 0;
    packet->temperature    = 0;
    packet->chargingStatus = DISP_CHARGING_STATUS_IDLE;
    memset(packet->vehicleId, ' ', DISP_VEHICLE_ID_SIZE);
    packet->saldo          = 0;
    packet->_saldoRsvd     = 0;
    memset(packet->platNomor, ' ', DISP_PLAT_NOMOR_SIZE);
    packet->crc16          = 0;
}

/**
 * Set identification fields (fixed-width ASCII, space-padded)
 * @param chargerBoxSerial   Charger box serial number  (max 8 chars)
 * @param chargerPointModel  Charger point model        (max 8 chars, e.g. "AC-007KW")
 * @param chargerPointSerial Charger point serial       (max 8 chars)
 * @param chargerPointVendor Vendor/tenant name         (max 8 chars, e.g. "QIMS-123")
 * @param firmwareVersion    Firmware version string    (max 16 chars, e.g. "V105.1.25.12")
 */
static inline void disp_setIdentification(DisplayPacket_t *packet,
                                           const char *chargerBoxSerial,
                                           const char *chargerPointModel,
                                           const char *chargerPointSerial,
                                           const char *chargerPointVendor,
                                           const char *firmwareVersion)
{
    disp_copyField(packet->chargerBoxSerial,   chargerBoxSerial,   DISP_CHARGER_BOX_SERIAL_SIZE);
    disp_copyField(packet->chargerPointModel,  chargerPointModel,  DISP_CHARGER_POINT_MODEL_SIZE);
    disp_copyField(packet->chargerPointSerial, chargerPointSerial, DISP_CHARGER_POINT_SERIAL_SIZE);
    disp_copyField(packet->chargerPointVendor, chargerPointVendor, DISP_CHARGER_POINT_VENDOR_SIZE);
    disp_copyField(packet->firmwareVersion,    firmwareVersion,    DISP_FIRMWARE_VERSION_SIZE);
}

/**
 * Set meter values and transaction-specific fields
 * @param connectorId    Connector ID (1-3)
 * @param energy         Energy in Wh
 * @param voltage        Voltage (V × 10)
 * @param current        Current (A × 10)
 * @param power          Power in W
 * @param temperature    Temperature (°C × 10)
 * @param chargingStatus Charging status (0=Idle, 1=Preparing, 2=Charging, 4=Fault)
 * @param transactionId  Pointer to 6-byte transaction ID
 * @param vehicleId      Vehicle ID string (max 16 chars)
 * @param saldo          Balance in Rupiah
 * @param platNomor      License plate string (max 14 chars)
 */
static inline void disp_setMeterValues(DisplayPacket_t *packet,
                                        uint8_t connectorId,
                                        uint32_t energy,
                                        uint16_t voltage,
                                        uint16_t current,
                                        uint32_t power,
                                        uint16_t temperature,
                                        uint8_t chargingStatus,
                                        const uint8_t *transactionId,
                                        const char *vehicleId,
                                        uint32_t saldo,
                                        const char *platNomor)
{
    packet->connectorId    = connectorId;
    packet->energy         = energy;
    packet->voltage        = voltage;
    packet->current        = current;
    packet->power          = power;
    packet->temperature    = temperature;
    packet->chargingStatus = chargingStatus;
    if (transactionId)
        memcpy(packet->transactionId, transactionId, sizeof(packet->transactionId));
    disp_copyField(packet->vehicleId, vehicleId,  DISP_VEHICLE_ID_SIZE);
    packet->saldo = saldo;
    disp_copyField(packet->platNomor, platNomor, DISP_PLAT_NOMOR_SIZE);
}

/**
 * Calculate CRC16 Modbus over bytes 0-112 and store result in packet->crc16
 */
static inline void disp_calculateCRC16(DisplayPacket_t *packet)
{
    uint16_t crc = 0xFFFF;
    uint8_t buffer[DISP_PACKET_SIZE];
    uint16_t len = 0;

    disp_serializePacket(packet, buffer, &len);
    len -= 2; // exclude crc16 field (bytes 113-114)

    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= buffer[pos];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    packet->crc16 = crc;
}

/**
 * Send display packet via Serial1 (UART1)
 */
static inline void disp_sendPacket(DisplayPacket_t *packet)
{
    uint8_t buffer[DISP_PACKET_SIZE];
    uint16_t len = 0;
    disp_serializePacket(packet, buffer, &len);
    for (uint16_t i = 0; i < len; i++)
        Serial1.write(buffer[i]);
}

#endif // DISPLAY_H
