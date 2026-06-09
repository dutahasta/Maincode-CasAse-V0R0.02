#ifndef HUB_H
#define HUB_H

#include <stdint.h>
#include <string.h>

// ===== RS485 HUB1 Communication Constants =====
#define HUB1_HEADER_BYTE1           0x5A
#define HUB1_HEADER_BYTE2           0xA5
#define HUB1_MESSAGE_TYPE_METER     0x0F

#define HUB1_SOURCE_ID              0x04  // MASTER
#define HUB1_DESTINATION_ID         0x01  // HUB1

// ===== Field Sizes =====
#define HUB1_CHARGER_BOX_SERIAL_SIZE        8
#define HUB1_CHARGER_POINT_MODEL_SIZE       8
#define HUB1_CHARGER_POINT_SERIAL_SIZE      8
#define HUB1_CHARGER_POINT_VENDOR_SIZE      8
#define HUB1_FIRMWARE_VERSION_SIZE          16

// ===== Packet Structure Offsets =====
#define HUB1_OFFSET_HEADER                  0   // Bytes 0-1   (2 bytes)
#define HUB1_OFFSET_LENGTH                  2   // Bytes 2-3   (2 bytes)
#define HUB1_OFFSET_MESSAGE_TYPE            4   // Byte 4
#define HUB1_OFFSET_SOURCE_ID               5   // Byte 5
#define HUB1_OFFSET_DESTINATION_ID          6   // Byte 6
#define HUB1_OFFSET_CHARGER_BOX_SERIAL      7   // Bytes 7-14  (8 bytes)
#define HUB1_OFFSET_CHARGER_POINT_MODEL     15  // Bytes 15-22 (8 bytes)
#define HUB1_OFFSET_CHARGER_POINT_SERIAL    23  // Bytes 23-30 (8 bytes)
#define HUB1_OFFSET_CHARGER_POINT_VENDOR    31  // Bytes 31-38 (8 bytes)
#define HUB1_OFFSET_FIRMWARE_VERSION        39  // Bytes 39-54 (16 bytes)
#define HUB1_OFFSET_CONNECTOR_ID            55  // Byte 55
#define HUB1_OFFSET_TRANSACTION_ID          56  // Bytes 56-61 (6 bytes)
#define HUB1_OFFSET_ENERGY                  62  // Bytes 62-65 (4 bytes)
#define HUB1_OFFSET_VOLTAGE                 66  // Bytes 66-67 (2 bytes)
#define HUB1_OFFSET_CURRENT                 68  // Bytes 68-69 (2 bytes)
#define HUB1_OFFSET_POWER                   70  // Bytes 70-73 (4 bytes)
#define HUB1_OFFSET_TEMPERATURE             74  // Bytes 74-75 (2 bytes)
#define HUB1_OFFSET_CHARGING_STATUS         76  // Byte 76
#define HUB1_OFFSET_CRC16                   77  // Bytes 77-78 (2 bytes)

#define HUB1_PACKET_SIZE                    79  // Total packet size

// ===== Charging Status Codes =====
#define HUB1_CHARGING_STATUS_IDLE       0x00
#define HUB1_CHARGING_STATUS_PREPARING  0x01
#define HUB1_CHARGING_STATUS_CHARGING   0x02
#define HUB1_CHARGING_STATUS_FAULT      0x03

// ===== Meter Value Packet Structure =====
typedef struct {
    uint8_t  header[2];                                                // Bytes 0-1
    uint16_t length;                                                   // Bytes 2-3
    uint8_t  messageType;                                              // Byte 4
    uint8_t  sourceId;                                                 // Byte 5
    uint8_t  destinationId;                                            // Byte 6
    char     chargerBoxSerial[HUB1_CHARGER_BOX_SERIAL_SIZE];           // Bytes 7-14
    char     chargerPointModel[HUB1_CHARGER_POINT_MODEL_SIZE];         // Bytes 15-22
    char     chargerPointSerial[HUB1_CHARGER_POINT_SERIAL_SIZE];       // Bytes 23-30
    char     chargerPointVendor[HUB1_CHARGER_POINT_VENDOR_SIZE];       // Bytes 31-38
    char     firmwareVersion[HUB1_FIRMWARE_VERSION_SIZE];              // Bytes 39-54
    uint8_t  connectorId;                                              // Byte 55
    uint8_t  transactionId[6];                                         // Bytes 56-61
    uint32_t energy;                                                   // Bytes 62-65 (Wh)
    uint16_t voltage;                                                  // Bytes 66-67 (V × 10)
    uint16_t current;                                                  // Bytes 68-69 (A × 10)
    uint32_t power;                                                    // Bytes 70-73 (W)
    uint16_t temperature;                                              // Bytes 74-75 (°C × 10)
    uint8_t  chargingStatus;                                           // Byte 76
    uint16_t crc16;                                                    // Bytes 77-78
} MeterValuePacket_t;

// Copy src string into fixed-width dest field, space-padded, no null terminator
static inline void hub1_copyField(char *dest, const char *src, uint8_t size)
{
    memset(dest, ' ', size);
    if (src) {
        uint8_t len = 0;
        while (src[len] && len < size) len++;
        memcpy(dest, src, len);
    }
}

// Serialize packet to raw byte buffer; sets *outLen to HUB1_PACKET_SIZE
static inline void hub1_serializePacket(const MeterValuePacket_t *packet, uint8_t *buffer, uint16_t *outLen)
{
    uint16_t i = 0;
    buffer[i++] = packet->header[0];
    buffer[i++] = packet->header[1];
    buffer[i++] = (uint8_t)(packet->length & 0xFF);
    buffer[i++] = (uint8_t)((packet->length >> 8) & 0xFF);
    buffer[i++] = packet->messageType;
    buffer[i++] = packet->sourceId;
    buffer[i++] = packet->destinationId;
    memcpy(&buffer[i], packet->chargerBoxSerial,   HUB1_CHARGER_BOX_SERIAL_SIZE);   i += HUB1_CHARGER_BOX_SERIAL_SIZE;
    memcpy(&buffer[i], packet->chargerPointModel,  HUB1_CHARGER_POINT_MODEL_SIZE);  i += HUB1_CHARGER_POINT_MODEL_SIZE;
    memcpy(&buffer[i], packet->chargerPointSerial, HUB1_CHARGER_POINT_SERIAL_SIZE); i += HUB1_CHARGER_POINT_SERIAL_SIZE;
    memcpy(&buffer[i], packet->chargerPointVendor, HUB1_CHARGER_POINT_VENDOR_SIZE); i += HUB1_CHARGER_POINT_VENDOR_SIZE;
    memcpy(&buffer[i], packet->firmwareVersion,    HUB1_FIRMWARE_VERSION_SIZE);     i += HUB1_FIRMWARE_VERSION_SIZE;
    buffer[i++] = packet->connectorId;
    memcpy(&buffer[i], packet->transactionId, 6); i += 6;
    buffer[i++] = (uint8_t)(packet->energy & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 8) & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 16) & 0xFF);
    buffer[i++] = (uint8_t)((packet->energy >> 24) & 0xFF);
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
    buffer[i++] = (uint8_t)(packet->crc16 & 0xFF);
    buffer[i++] = (uint8_t)((packet->crc16 >> 8) & 0xFF);
    if (outLen) *outLen = i;
}

/**
 * Initialize meter value packet with default header and IDs
 */
static inline void hub1_initMeterPacket(MeterValuePacket_t *packet)
{
    packet->header[0]     = HUB1_HEADER_BYTE1;
    packet->header[1]     = HUB1_HEADER_BYTE2;
    packet->length        = HUB1_PACKET_SIZE - 4;
    packet->messageType   = HUB1_MESSAGE_TYPE_METER;
    packet->sourceId      = HUB1_SOURCE_ID;
    packet->destinationId = HUB1_DESTINATION_ID;
    memset(packet->chargerBoxSerial,   ' ', HUB1_CHARGER_BOX_SERIAL_SIZE);
    memset(packet->chargerPointModel,  ' ', HUB1_CHARGER_POINT_MODEL_SIZE);
    memset(packet->chargerPointSerial, ' ', HUB1_CHARGER_POINT_SERIAL_SIZE);
    memset(packet->chargerPointVendor, ' ', HUB1_CHARGER_POINT_VENDOR_SIZE);
    memset(packet->firmwareVersion,    ' ', HUB1_FIRMWARE_VERSION_SIZE);
    packet->connectorId    = 1;
    memset(packet->transactionId, 0, sizeof(packet->transactionId));
    packet->energy         = 0;
    packet->voltage        = 0;
    packet->current        = 0;
    packet->power          = 0;
    packet->temperature    = 0;
    packet->chargingStatus = HUB1_CHARGING_STATUS_IDLE;
    packet->crc16          = 0;
}

/**
 * Set identification fields (fixed-width ASCII, space-padded)
 * @param chargerBoxSerial   Charger box serial number  (max 8 chars, e.g. "00012345")
 * @param chargerPointModel  Charger point model        (max 8 chars, e.g. "AC-007KW")
 * @param chargerPointSerial Charger point serial       (max 8 chars, e.g. "00012345")
 * @param chargerPointVendor Vendor/tenant name         (max 8 chars, e.g. "QIMS-123")
 * @param firmwareVersion    Firmware version string    (max 16 chars, e.g. "V105.1.25.12")
 */
static inline void hub1_setIdentification(MeterValuePacket_t *packet,
                                           const char *chargerBoxSerial,
                                           const char *chargerPointModel,
                                           const char *chargerPointSerial,
                                           const char *chargerPointVendor,
                                           const char *firmwareVersion)
{
    hub1_copyField(packet->chargerBoxSerial,   chargerBoxSerial,   HUB1_CHARGER_BOX_SERIAL_SIZE);
    hub1_copyField(packet->chargerPointModel,  chargerPointModel,  HUB1_CHARGER_POINT_MODEL_SIZE);
    hub1_copyField(packet->chargerPointSerial, chargerPointSerial, HUB1_CHARGER_POINT_SERIAL_SIZE);
    hub1_copyField(packet->chargerPointVendor, chargerPointVendor, HUB1_CHARGER_POINT_VENDOR_SIZE);
    hub1_copyField(packet->firmwareVersion,    firmwareVersion,    HUB1_FIRMWARE_VERSION_SIZE);
}

/**
 * Set meter value data in packet
 * @param connectorId     Connector ID (1-3)
 * @param energy          Energy in Wh
 * @param voltage         Voltage (V × 10)
 * @param current         Current (A × 10)
 * @param power           Power in W
 * @param temperature     Temperature (°C × 10)
 * @param chargingStatus  Charging status (0-3)
 * @param transactionId   Pointer to 6-byte transaction ID
 */
static inline void hub1_setMeterValues(MeterValuePacket_t *packet,
                                        uint8_t connectorId,
                                        uint32_t energy,
                                        uint16_t voltage,
                                        uint16_t current,
                                        uint32_t power,
                                        uint16_t temperature,
                                        uint8_t chargingStatus,
                                        uint8_t *transactionId)
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
}

/**
 * Calculate CRC16 Modbus over bytes 0-76 and store result in packet->crc16
 */
static inline void hub1_calculateCRC16(MeterValuePacket_t *packet)
{
    uint16_t crc = 0xFFFF;
    uint8_t buffer[HUB1_PACKET_SIZE];
    uint16_t len = 0;

    hub1_serializePacket(packet, buffer, &len);
    len -= 2; // exclude crc16 field (bytes 77-78)

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
 * Print packet bytes as hexadecimal to Serial
 */
static inline void hub1_printMeterPacket(MeterValuePacket_t *packet)
{
    uint8_t buffer[HUB1_PACKET_SIZE];
    uint16_t len = 0;
    hub1_serializePacket(packet, buffer, &len);
    for (uint16_t i = 0; i < len; i++) {
        if (buffer[i] < 0x10)
            Serial.print('0');
        Serial.print(buffer[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}

/**
 * Send meter value packet to HUB1 via RS485
 * @return 1 if success, 0 if failed
 */
static inline uint8_t hub1_sendMeterPacket(MeterValuePacket_t *packet)
{
    (void)packet;
    return 0;
}

#endif // HUB_H
