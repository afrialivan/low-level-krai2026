#include "MotorControl.h"
#include "Config.h"
#include <Arduino.h>

// Inisialisasi pin motor lokal
MotorData motTR = {FR_PWM, FR_IN1, FR_IN2, 0};
MotorData motTL = {FL_PWM, FL_IN1, FL_IN2, 0};

void setupMotors() {
    MotorData *motors[] = { &motTR, &motTL };
    for(int i = 0; i < 2; i++) {
        pinMode(motors[i]->pwmPin, OUTPUT); 
        pinMode(motors[i]->in1, OUTPUT); 
        pinMode(motors[i]->in2, OUTPUT);
        putarMotor(*motors[i], 0); // Pastikan motor mati di awal
    }
}

void putarMotor(MotorData &m, int kecepatan) {
    m.speed = kecepatan; // Simpan nilai untuk dikirim ke ROS jika diperlukan
    int nilai_pwm = constrain(abs(kecepatan), 0, 255);
    
    if (kecepatan > 0) { 
        digitalWrite(m.in1, HIGH); 
        digitalWrite(m.in2, LOW); 
        analogWrite(m.pwmPin, nilai_pwm); 
    }
    else if (kecepatan < 0) { 
        digitalWrite(m.in1, LOW); 
        digitalWrite(m.in2, HIGH); 
        analogWrite(m.pwmPin, nilai_pwm); 
    }
    else { 
        digitalWrite(m.in1, LOW); 
        digitalWrite(m.in2, LOW); 
        analogWrite(m.pwmPin, 0); 
    }
}