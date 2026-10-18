#include <Arduino.h>
#include "Config.h"
#include "Encoder.h"
#include "MotorControl.h"

#ifdef USE_ESP_NOW
  #include "Communication.h"
#endif

#ifdef USE_UART
  #include "UartCommunication.h"
#endif

unsigned long lastTime = 0;
float dt = 0.1; // Loop PID jalan tiap 100ms

void setup() {
  Serial.begin(115200);

  setupEncoders();
  setupMotors();

  #ifdef USE_ESP_NOW
    setupESPNow();
    Serial.println("Mode Komunikasi: ESP-NOW");
  #endif

  #ifdef USE_UART
    setupUART();
    Serial.println("Mode Komunikasi: UART (Serial2)");
  #endif

  lastTime = millis();
}

void loop() {
  unsigned long now = millis();

  #ifdef USE_UART
    processUART();
  #endif

  if (now - lastTime >= 100) {
    long deltaFL = getEncoderDelta(encFL);
    long deltaFR = getEncoderDelta(encFR);
    long deltaRL = getEncoderDelta(encRL);
    long deltaRR = getEncoderDelta(encRR);

    updateMotorPID(motFL, deltaFL, dt);
    updateMotorPID(motFR, deltaFR, dt);
    updateMotorPID(motRL, deltaRL, dt);
    updateMotorPID(motRR, deltaRR, dt);

    #ifdef USE_ESP_NOW
      sendFeedback();
    #endif

    #ifdef USE_UART
      sendFeedbackUART();
    #endif

    // Debugging RPM ke Serial Monitor (PC)
    Serial.printf("RPM FR: %d | FL: %d | RR: %d | RL: %d\n", 
                  motFR.setpoint, motFL.setpoint, motRR.setpoint, motRL.setpoint);


    lastTime = now;
  }
}