#include "UartCommunication.h"
#include "Config.h"
#include "MotorControl.h"
#include "IMU.h"
#include <Arduino.h>

#define UART_BAUDRATE 115200

void setupUART() {
    // Default RX = 16, TX = 17
    Serial2.begin(UART_BAUDRATE);
}

void sendToSlaveUART(int fl, int fr, int rl, int rr) {
    // Kirim data ke Slave format: FL,FR,RL,RR\n
    Serial2.printf("%d,%d,%d,%d\n", fl, fr, rl, rr);
}

void processUARTFeedback() {
    if (Serial2.available() > 0) {
        String data = Serial2.readStringUntil('\n');
        float rpmFL, rpmFR, rpmRL, rpmRR;
        long countFL, countFR, countRL, countRR;
        
        // Parsing data dari Slave (Sesuai dengan format yang dikirim Slave)
        if (sscanf(data.c_str(), "%f,%f,%f,%f,%ld,%ld,%ld,%ld", 
                   &rpmFL, &rpmFR, &rpmRL, &rpmRR, 
                   &countFL, &countFR, &countRL, &countRR) == 8) {
            
            // Gabungkan dengan data lokal & IMU, kirim ke ROS
            Serial.printf("FEEDBACK:%.2f,%.2f,%.2f,%.2f,%d,%d,%ld,%ld,%ld,%ld,0,0,%.2f\n",
                          rpmFL, rpmFR, rpmRL, rpmRR,
                          motTL.speed, motTR.speed,
                          countFL, countFR, countRL, countRR,
                          getYawIMU());
        }
    }
}