#ifndef CONFIG_H
#define CONFIG_H

// PWM configuration
#define PWM_PIN   12
#define PWM_FREQ 1000
#define PWM_CH   0
#define PWM_RES  8

// DISPLAY UART configuration
#define DISPLAY_UART_BAUD 115200
#define DISPLAY_UART_RX   14
#define DISPLAY_UART_TX   27

// PZEM configuration
#define PZEM_RX 16
#define PZEM_TX 17
#define PZEM_DE 13

// Relay pins
#define RELAY1    25
#define RELAY2    26

// LED pins
#define LED1      32
#define LED2      33

// Temperature sensor pin
#define ONE_WIRE_BUS 15

#endif // CONFIG_H