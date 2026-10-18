#include <Arduino.h>
#include "Config.h"
#include "IMU.h"
#include "MotorControl.h"

#ifdef USE_ESP_NOW
#include "Communication.h"
#endif

#ifdef USE_UART
#include "UartCommunication.h"
#endif

#ifdef USE_ESP3_UART
#include "UartESP3.h"
#endif

void setup() {
  Serial.begin(115200);

  // setupIMU();
  setupMotors();

  #ifdef USE_ESP_NOW
    setupESPNow();
  #endif

  #ifdef USE_UART
    setupUART();
  #endif

  // Setup UART untuk ESP ke-3
  #ifdef USE_ESP3_UART
    setupESP3UART();
  #endif
}

void processROSCommand() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    float v[6];
    int idx = 0, pos = 0;

    while ((pos = data.indexOf(',')) != -1 && idx < 5) {
      v[idx++] = data.substring(0, pos).toFloat();
      data = data.substring(pos + 1);
    }
    v[idx] = data.toFloat();

    putarMotor(motTL, (int)v[4]);
    putarMotor(motTR, (int)v[5]);

    #ifdef USE_ESP_NOW
        sendToSlaveESPNow((int)v[0], (int)v[1], (int)v[2], (int)v[3]);
    #endif

    #ifdef USE_UART
        sendToSlaveUART((int)v[0], (int)v[1], (int)v[2], (int)v[3]);
    #endif

    #ifdef USE_ESP3_UART
        // Ambil data ke-6 dan ke-7 dari array ROS, lalu kirim ke ESP ke-3
        int perintah_untuk_esp3 = (int)v[6];
        float parameter_untuk_esp3 = v[7];

        sendToESP3(perintah_untuk_esp3, parameter_untuk_esp3);
    #endif
  }
}

void loop() {
  // updateIMU();
  processROSCommand();

  #ifdef USE_UART
    processUARTFeedback();
  #endif

  // Baca data dari ESP ke-3 secara terus menerus (Polling)
  #ifdef USE_ESP3_UART
    processESP3Data();
  #endif
}