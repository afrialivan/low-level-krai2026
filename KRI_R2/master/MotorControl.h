#pragma once

struct MotorData {
    int pwmPin;
    int in1;
    int in2;
    int speed; // Menyimpan nilai kecepatan (PWM) saat ini
};

extern MotorData motTR;
extern MotorData motTL;

void setupMotors();
void putarMotor(MotorData &m, int kecepatan);