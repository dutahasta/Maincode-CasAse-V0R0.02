#ifndef HUB_H
#define HUB_H

#include <stdint.h>
#include <string.h>

// ===== RS485 HUB1 Communication Constants =====
#define HUB1_HEADER_BYTE1       0x5A
#define HUB1_HEADER_BYTE2       0xA5
#define HUB1_MESSAGE_TYPE_METER 0x0F

#define HUB1_SOURCE_ID          0x04  // MASTER
#define HUB1_DESTINATION_ID     0x01  // HUB1

// ===== Packet Structure Offsets =====
#define HUB1_OFFSET_HEADER          0      // Bytes 0-1
#define HUB1_OFFSET_LENGTH          2      // Bytes 2-3
#define HUB1_OFFSET_MESSAGE_TYPE    4      // Byte 4
#define HUB1_OFFSET_SOURCE_ID       5      // Byte 5
#define HUB1_OFFSET_DESTINATION_ID  6      // Byte 6
#define HUB1_OFFSET_CONNECTOR_ID    7      // Byte 7
#define HUB1_OFFSET_TRANSACTION_ID  8      // Bytes 8-13 (6 bytes)
#define HUB1_OFFSET_ENERGY          14     // Bytes 14-17 (4 bytes)
#define HUB1_OFFSET_VOLTAGE         18     // Bytes 18-19 (2 bytes)
#define HUB1_OFFSET_CURRENT         20     // Bytes 20-21 (2 bytes)
#define HUB1_OFFSET_POWER           22     // Bytes 22-25 (4 bytes)
#define HUB1_OFFSET_TEMPERATURE     26     // Bytes 26-27 (2 bytes)
#define HUB1_OFFSET_CHARGING_STATUS 28     // Byte 28
#define HUB1_OFFSET_CRC16           29     // Bytes 29-30 (2 bytes)

#define HUB1_PACKET_SIZE            31     // Total packet size

// ===== Charging Status Codes =====
#define HUB1_CHARGING_STATUS_IDLE      0x00
#define HUB1_CHARGING_STATUS_PREPARING 0x01
#define HUB1_CHARGING_STATUS_CHARGING  0x02
#define HUB1_CHARGING_STATUS_FAULT     0x03

// ===== Meter Value Packet Structure =====
typedef struct {
    uint8_t  header[2];              // Bytes 0-1: Header (0x5A, 0xA5)
    uint16_t length;                 // Bytes 2-3: Length of data
    uint8_t  messageType;            // Byte 4: Message Type (0x0F)
    uint8_t  sourceId;               // Byte 5: Source ID (0x04 = MASTER)
    uint8_t  destinationId;          // Byte 6: Destination ID (0x01 = HUB1)
    uint8_t  connectorId;            // Byte 7: Connector ID (1-3)
    uint8_t  transactionId[6];       // Bytes 8-13: Transaction ID
    uint32_t energy;                 // Bytes 14-17: Energy (Wh)
    uint16_t voltage;                // Bytes 18-19: Voltage (V × 10)
    uint16_t current;                // Bytes 20-21: Current (A × 10)
    uint32_t power;                  // Bytes 22-25: Power (W)
    uint16_t temperature;            // Bytes 26-27: Temperature (°C × 10)
    uint8_t  chargingStatus;         // Byte 28: Charging Status
    uint16_t crc16;                  // Bytes 29-30: CRC16 Modbus
} MeterValuePacket_t;

/**
 * Initialize meter value packet with default header and IDs
 * @param packet Pointer to MeterValuePacket_t structure
 */
static inline void hub1_initMeterPacket(MeterValuePacket_t *packet)
{
  packet->header[0] = HUB1_HEADER_BYTE1;
  packet->header[1] = HUB1_HEADER_BYTE2;
  packet->length = HUB1_PACKET_SIZE - 4;
  packet->messageType = HUB1_MESSAGE_TYPE_METER;
  packet->sourceId = HUB1_SOURCE_ID;
  packet->destinationId = HUB1_DESTINATION_ID;
  packet->connectorId = 1;
  memset(packet->transactionId, 0, sizeof(packet->transactionId));
  packet->energy = 0;
  packet->voltage = 0;
  packet->current = 0;
  packet->power = 0;
  packet->temperature = 0;
  packet->chargingStatus = HUB1_CHARGING_STATUS_IDLE;
  packet->crc16 = 0;
}

/**
 * Set meter value data in packet
 * @param packet Pointer to MeterValuePacket_t structure
 * @param connectorId Connector ID (1-3)
 * @param energy Energy in Wh
 * @param voltage Voltage (V × 10)
 * @param current Current (A × 10)
 * @param power Power in W
 * @param temperature Temperature (°C × 10)
 * @param chargingStatus Charging status (0-3)
 * @param transactionId Pointer to 6-byte transaction ID
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
  packet->connectorId = connectorId;
  packet->energy = energy;
  packet->voltage = voltage;
  packet->current = current;
  packet->power = power;
  packet->temperature = temperature;
  packet->chargingStatus = chargingStatus;

  if (transactionId)
  {
    memcpy(packet->transactionId, transactionId, sizeof(packet->transactionId));
  }
}

/**
 * Calculate CRC16 Modbus for packet
 * @param packet Pointer to MeterValuePacket_t structure
 */
static inline void hub1_calculateCRC16(MeterValuePacket_t *packet)
{
  uint16_t crc = 0xFFFF;
  uint8_t buffer[HUB1_PACKET_SIZE];
  uint16_t index = 0;

  buffer[index++] = packet->header[0];
  buffer[index++] = packet->header[1];
  buffer[index++] = packet->length & 0xFF;
  buffer[index++] = (packet->length >> 8) & 0xFF;
  buffer[index++] = packet->messageType;
  buffer[index++] = packet->sourceId;
  buffer[index++] = packet->destinationId;
  buffer[index++] = packet->connectorId;
  memcpy(&buffer[index], packet->transactionId, sizeof(packet->transactionId));
  index += sizeof(packet->transactionId);
  buffer[index++] = packet->energy & 0xFF;
  buffer[index++] = (packet->energy >> 8) & 0xFF;
  buffer[index++] = (packet->energy >> 16) & 0xFF;
  buffer[index++] = (packet->energy >> 24) & 0xFF;
  buffer[index++] = packet->voltage & 0xFF;
  buffer[index++] = (packet->voltage >> 8) & 0xFF;
  buffer[index++] = packet->current & 0xFF;
  buffer[index++] = (packet->current >> 8) & 0xFF;
  buffer[index++] = packet->power & 0xFF;
  buffer[index++] = (packet->power >> 8) & 0xFF;
  buffer[index++] = (packet->power >> 16) & 0xFF;
  buffer[index++] = (packet->power >> 24) & 0xFF;
  buffer[index++] = packet->temperature & 0xFF;
  buffer[index++] = (packet->temperature >> 8) & 0xFF;
  buffer[index++] = packet->chargingStatus;

  for (uint16_t pos = 0; pos < index; pos++)
  {
    crc ^= buffer[pos];
    for (uint8_t i = 0; i < 8; i++)
    {
      if (crc & 0x0001)
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  packet->crc16 = crc;
}

/**
 * Print packet bytes as hexadecimal to Serial
 * @param packet Pointer to MeterValuePacket_t structure
 */
static inline void hub1_printMeterPacket(MeterValuePacket_t *packet)
{
  for (int i = 0; i < HUB1_PACKET_SIZE; i++)
  {
    uint8_t b;

    if (i < 2)
      b = packet->header[i];
    else if (i < 4)
      b = (i == 2) ? (uint8_t)(packet->length & 0xFF) : (uint8_t)((packet->length >> 8) & 0xFF);
    else if (i == 4)
      b = packet->messageType;
    else if (i == 5)
      b = packet->sourceId;
    else if (i == 6)
      b = packet->destinationId;
    else if (i == 7)
      b = packet->connectorId;
    else if (i < 14)
      b = packet->transactionId[i - 8];
    else if (i < 18)
      b = (i == 14) ? (uint8_t)(packet->energy & 0xFF) : (i == 15) ? (uint8_t)((packet->energy >> 8) & 0xFF) : (i == 16) ? (uint8_t)((packet->energy >> 16) & 0xFF) : (uint8_t)((packet->energy >> 24) & 0xFF);
    else if (i < 20)
      b = (i == 18) ? (uint8_t)(packet->voltage & 0xFF) : (uint8_t)((packet->voltage >> 8) & 0xFF);
    else if (i < 22)
      b = (i == 20) ? (uint8_t)(packet->current & 0xFF) : (uint8_t)((packet->current >> 8) & 0xFF);
    else if (i < 26)
      b = (i == 22) ? (uint8_t)(packet->power & 0xFF) : (i == 23) ? (uint8_t)((packet->power >> 8) & 0xFF) : (i == 24) ? (uint8_t)((packet->power >> 16) & 0xFF) : (uint8_t)((packet->power >> 24) & 0xFF);
    else if (i < 28)
      b = (i == 26) ? (uint8_t)(packet->temperature & 0xFF) : (uint8_t)((packet->temperature >> 8) & 0xFF);
    else if (i == 28)
      b = packet->chargingStatus;
    else if (i == 29)
      b = (uint8_t)(packet->crc16 & 0xFF);
    else
      b = (uint8_t)((packet->crc16 >> 8) & 0xFF);

    if (b < 0x10)
      Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();
}

/**
 * Send meter value packet to HUB1 via RS485
 * @param packet Pointer to MeterValuePacket_t structure
 * @return 1 if success, 0 if failed
 */
static inline uint8_t hub1_sendMeterPacket(MeterValuePacket_t *packet)
{
  (void)packet;
  return 0;
}

#endif // HUB_H
