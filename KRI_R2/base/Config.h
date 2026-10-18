#pragma once

// ==========================================
// PILIH MODE KOMUNIKASI (Buka komentar pada salah satu)
// ==========================================
#define USE_ESP_NOW
// #define USE_UART

// ==========================================
// KONFIGURASI PIN
// ==========================================
// Pin Motor & Encoder RL
constexpr int RL_PWM = 18;
constexpr int RL_IN1 = 4;
constexpr int RL_IN2 = 5;
constexpr int RL_ENC_A = 34;
constexpr int RL_ENC_B = 35;

// Pin Motor & Encoder RR
constexpr int RR_PWM = 19;
constexpr int RR_IN1 = 2;
constexpr int RR_IN2 = 23;
constexpr int RR_ENC_A = 39;
constexpr int RR_ENC_B = 36;

// Pin Motor & Encoder FR
constexpr int FR_PWM = 21;
constexpr int FR_IN1 = 27;
constexpr int FR_IN2 = 26;
constexpr int FR_ENC_A = 25;
constexpr int FR_ENC_B = 15;

// Pin Motor & Encoder FL
constexpr int FL_PWM = 22;
constexpr int FL_IN1 = 14;
constexpr int FL_IN2 = 13;
constexpr int FL_ENC_A = 32;
constexpr int FL_ENC_B = 33;