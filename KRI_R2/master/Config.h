#pragma once
#include <Arduino.h>

#define USE_ESP_NOW
// #define USE_UART

#define USE_ESP3_UART

#ifdef USE_ESP3_UART
  constexpr int ESP3_RX = 27; 
  constexpr int ESP3_TX = 26; 
#endif

constexpr int FR_PWM = 18;
constexpr int FR_IN1 = 4;
constexpr int FR_IN2 = 2;

constexpr int FL_PWM = 19;
constexpr int FL_IN1 = 5;
constexpr int FL_IN2 = 23;

// MAC Address Slave (Untuk ESP-NOW)
const uint8_t slaveAddress[] = {0x20, 0xE7, 0xC8, 0xA7, 0x9B, 0x0C};