#pragma once
#include <Arduino.h>

struct MotorData {
  int pwmPin;
  int in1;
  int in2;
  int encA;
  int encB;
  volatile long encoderCount;
  volatile int lastEncoded;
  long lastCount;
  float ppr;
  float setpoint;
  float rpm;
  float Kp;
  float Ki;
  float Kd;
  float error;
  float lastError;
  float integral;
  float derivative;
  float control;
  int pwmOutput;
};

extern MotorData motRL, motRR, motFR, motFL;

void setupMotors();
void putarMotor(MotorData &m, int kecepatan);
void updateMotorPID(MotorData &m, long deltaPulsa, float dt); // Motor hanya butuh delta pulsa