#include "Encoder.h"
#include "Config.h"

// Inisialisasi struct Encoder
EncoderData encRL = { RL_ENC_A, RL_ENC_B, 0, 0, 0 };
EncoderData encRR = { RR_ENC_A, RR_ENC_B, 0, 0, 0 };
EncoderData encFR = { FR_ENC_A, FR_ENC_B, 0, 0, 0 };
EncoderData encFL = { FL_ENC_A, FL_ENC_B, 0, 0, 0 };

// Fungsi internal untuk mengupdate nilai tiap encoder
void updateEncoderState(EncoderData &enc) {
  int A = digitalRead(enc.pinA);
  int B = digitalRead(enc.pinB);
  int encoded = (A << 1) | B;
  int sum = (enc.lastState << 2) | encoded;
  
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) enc.count++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) enc.count--;
  
  enc.lastState = encoded;
}

// ISR Handlers
void IRAM_ATTR isrRL() { updateEncoderState(encRL); }
void IRAM_ATTR isrRR() { updateEncoderState(encRR); }
void IRAM_ATTR isrFR() { updateEncoderState(encFR); }
void IRAM_ATTR isrFL() { updateEncoderState(encFL); }

void setupEncoders() {
  pinMode(encRL.pinA, INPUT); pinMode(encRL.pinB, INPUT);
  pinMode(encRR.pinA, INPUT); pinMode(encRR.pinB, INPUT);
  pinMode(encFR.pinA, INPUT); pinMode(encFR.pinB, INPUT);
  pinMode(encFL.pinA, INPUT); pinMode(encFL.pinB, INPUT);

  attachInterrupt(digitalPinToInterrupt(encRL.pinA), isrRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encRL.pinB), isrRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encRR.pinA), isrRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encRR.pinB), isrRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encFR.pinA), isrFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encFR.pinB), isrFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encFL.pinA), isrFL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encFL.pinB), isrFL, CHANGE);
}

// Fungsi ini mengembalikan selisih pulsa dari pembacaan terakhir
long getEncoderDelta(EncoderData &enc) {
  long currentCount = enc.count;
  long delta = currentCount - enc.lastCount;
  enc.lastCount = currentCount;
  return delta;
}