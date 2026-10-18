#include "IMU.h"
#include <Wire.h>
#include "MPU9250.h"

MPU9250 mpu;

void setupIMU() {
    Wire.begin();
    delay(2000); // Beri waktu I2C untuk stabil
    if (!mpu.setup(0x68)) {
        while (1) { 
            Serial.println("MPU Fail"); 
            delay(5000); 
        }
    }
    Serial.println("Calibrating... Do not move");
    mpu.calibrateAccelGyro();
}

void updateIMU() {
    mpu.update();
}

float getYawIMU() {
    return mpu.getYaw();
}