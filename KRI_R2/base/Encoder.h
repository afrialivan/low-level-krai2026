#pragma once
#include <Arduino.h>

struct EncoderData {
  int pinA;
  int pinB;
  volatile long count;
  volatile int lastState;
  long lastCount; // Untuk menghitung selisih (delta) tiap loop
};

extern EncoderData encRL;
extern EncoderData encRR;
extern EncoderData encFR;
extern EncoderData encFL;

void setupEncoders();
long getEncoderDelta(EncoderData &enc); // Mengambil selisih pulsa untuk perhitungan RPM