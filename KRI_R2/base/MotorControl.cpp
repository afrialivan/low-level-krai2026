#include "MotorControl.h"
#include "Config.h"

// Inisialisasi Objek Motor
MotorData motRL = { RL_PWM, RL_IN1, RL_IN2, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0 };
MotorData motRR = { RR_PWM, RR_IN1, RR_IN2, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0 };
MotorData motFR = { FR_PWM, FR_IN1, FR_IN2, 40600, 0, 0, 2.1, 2.5, 0.03, 0, 0, 0 };
MotorData motFL = { FL_PWM, FL_IN1, FL_IN2, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0 };

void setupMotors() {
  MotorData *motors[] = { &motRL, &motRR, &motFR, &motFL };
  for (int i = 0; i < 4; i++) {
    pinMode(motors[i]->pwmPin, OUTPUT);
    pinMode(motors[i]->in1, OUTPUT);
    pinMode(motors[i]->in2, OUTPUT);
    putarMotor(*motors[i], 0); // Pastikan motor mati saat awal
  }
}

void putarMotor(MotorData &m, int kecepatan) {
  int nilai_pwm = constrain(abs(kecepatan), 0, 255);
  if (kecepatan > 0) {
    digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, nilai_pwm);
  } else if (kecepatan < 0) {
    digitalWrite(m.in1, LOW); digitalWrite(m.in2, HIGH); analogWrite(m.pwmPin, nilai_pwm);
  } else {
    digitalWrite(m.in1, LOW); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, 0);
  }
}

void updateMotorPID(MotorData &m, long deltaPulsa, float dt) {
  // Hitung RPM berdasarkan delta pulsa yang dikirim dari luar
  m.rpm = (deltaPulsa * 600.0) / m.ppr; 

  m.error = abs(m.setpoint) - abs(m.rpm);
  m.integral = constrain(m.integral + (m.error * dt), -500, 500);
  float derivative = (m.error - m.lastError) / dt;
  float control = (m.Kp * m.error) + (m.Ki * m.integral) + (m.Kd * derivative);
  int pwmOutput = constrain(abs(control), 0, 255);

  if (m.setpoint > 0) putarMotor(m, pwmOutput);
  else if (m.setpoint < 0) putarMotor(m, -pwmOutput);
  else {
    putarMotor(m, 0);
    m.integral = 0;
  }
  m.lastError = m.error;
}