#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Library untuk DMP MPU6050
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// Pin Ultrasonik 1
#define TRIG_PIN 13
#define ECHO_PIN 14

// Pin Ultrasonik 2 
#define TRIG2_PIN 16
#define ECHO2_PIN 17 

#define TRIG3_PIN 18
#define ECHO3_PIN 19

#define TRIG4_PIN 4
#define ECHO4_PIN 23

#define PROX_PIN 33 // proxy 4
#define PROX2_PIN 25  // proxy 2
#define PROX3_PIN 26 // proxy 1
#define PHOTO_PIN 32 

volatile float distance = 0.0; 
volatile float distance2 = 0.0; 
volatile float distance3 = 0.0; // [TAMBAHAN] Variabel Jarak Ultrasonik 3
volatile float distance4 = 0.0; 

volatile int proxState = 1;
volatile int prox2State = 1;  
volatile int prox3State = 1;    // [TAMBAHAN] Variabel Proximity 3
volatile int lightState = 0; 

// ======================================================
// VARIABEL MPU6050 & DMP
// ======================================================
MPU6050 mpu;
bool dmpReady = false;  
uint8_t devStatus;      
uint16_t packetSize;    
uint8_t fifoBuffer[64]; 

Quaternion q;           // [w, x, y, z] Vektor Kuaternion
VectorFloat gravity;    // [x, y, z] Vektor Gravitasi
float ypr[3];           // [yaw, pitch, roll] 

// Variabel Global untuk Yaw, Pitch, Roll
float robot_yaw = 0.0f;
float robot_pitch = 0.0f;
float robot_roll = 0.0f;

// Mutex untuk mengamankan pembacaan dan penulisan data IMU
SemaphoreHandle_t imuMutex;

// ======================================================
// TASK 1: Sensor Ultrasonik 1, 2, 3 & 4
// ======================================================
void taskUltrasonic(void *parameter) {
  for (;;) {
    // Membaca Ultrasonik 1
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration1 = pulseIn(ECHO_PIN, HIGH, 20000);
    if (duration1 == 0) {
      distance = -1.0; 
    } else {
      distance = duration1 * 0.034 / 2;
    }

    // Jeda singkat agar sinyal ultrasonik 1 tidak dibaca oleh ultrasonik 2
    vTaskDelay(pdMS_TO_TICKS(10));

    // Membaca Ultrasonik 2
    digitalWrite(TRIG2_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG2_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG2_PIN, LOW);
    
    long duration2 = pulseIn(ECHO2_PIN, HIGH, 20000);
    if (duration2 == 0) {
      distance2 = -1.0; 
    } else {
      distance2 = duration2 * 0.034 / 2;
    }

    // [TAMBAHAN] Jeda singkat agar sinyal ultrasonik 2 tidak dibaca oleh ultrasonik 3
    vTaskDelay(pdMS_TO_TICKS(10));

    // [TAMBAHAN] Membaca Ultrasonik 3
    digitalWrite(TRIG3_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG3_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG3_PIN, LOW);
    
    long duration3 = pulseIn(ECHO3_PIN, HIGH, 20000);
    if (duration3 == 0) {
      distance3 = -1.0; 
    } else {
      distance3 = duration3 * 0.034 / 2;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    // Membaca Ultrasonik 4
    digitalWrite(TRIG4_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG4_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG4_PIN, LOW);
    
    long duration4 = pulseIn(ECHO4_PIN, HIGH, 20000);
    if (duration4 == 0) {
      distance4 = -1.0; 
    } else {
      distance4 = duration4 * 0.034 / 2;
    }

    vTaskDelay(pdMS_TO_TICKS(15));
  }
}

// ======================================================
// TASK 2: Sensor Proximity 1, 2, & 3
// ======================================================
void taskProximity(void *parameter) {
  for (;;) {
    proxState = digitalRead(PROX_PIN);
    prox2State = digitalRead(PROX2_PIN); 
    prox3State = digitalRead(PROX3_PIN); // [TAMBAHAN] Baca Proximity 3
    vTaskDelay(pdMS_TO_TICKS(15));
  }
}

// ======================================================
// TASK 3: Sensor Photodiode
// ======================================================
void taskPhotodiode(void *parameter) {
  for (;;) {
    int rawLight = analogRead(PHOTO_PIN);
    if (rawLight < 40) {
      lightState = 0;
    } else if(rawLight > 50){
      lightState = 1;
    }
    vTaskDelay(pdMS_TO_TICKS(15)); 
  }
}

// ======================================================
// TASK 4: MPU6050 (DMP)
// ======================================================
void taskMPU6050(void *parameter) {
  mpu.initialize();
  
  if(!mpu.testConnection()) {
      Serial.println("MPU6050 koneksi gagal. Task IMU dihentikan.");
      vTaskDelete(NULL);
  }

  devStatus = mpu.dmpInitialize();

  // Offset Kalibrasi (Bisa diubah sesuai hasil kalibrasi MPU6050 Anda)
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);

  if (devStatus == 0) {
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      mpu.setDMPEnabled(true);
      dmpReady = true;
      packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
      Serial.print("DMP Error: "); Serial.println(devStatus);
      vTaskDelete(NULL);
  }

  for (;;) {
      if (!dmpReady) {
          vTaskDelay(pdMS_TO_TICKS(10));
          continue;
      }

      // Baca FIFO Buffer DMP terbaru
      if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {  
          mpu.dmpGetQuaternion(&q, fifoBuffer);
          mpu.dmpGetGravity(&gravity, &q);
          mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

          // Simpan data dengan aman menggunakan Mutex
          if (xSemaphoreTake(imuMutex, portMAX_DELAY) == pdTRUE) {
              robot_yaw = ypr[0] * 180.0 / M_PI;
              robot_pitch = ypr[1] * 180.0 / M_PI;
              robot_roll = ypr[2] * 180.0 / M_PI;
              xSemaphoreGive(imuMutex);
          }
      }
      vTaskDelay(pdMS_TO_TICKS(10)); // Eksekusi dengan interval ~10ms (100Hz)
  }
}

// ======================================================
// TASK 5: Print Serial (Ke ROS)
// ======================================================
void taskPrint(void *parameter) {
  for (;;) {
    // Variabel lokal untuk menampung data IMU secara temporer
    float curr_yaw = 0.0f;
    float curr_pitch = 0.0f;
    float curr_roll = 0.0f;

    // Ambil data IMU dengan aman dari Mutex
    if (xSemaphoreTake(imuMutex, portMAX_DELAY) == pdTRUE) {
        curr_yaw = robot_yaw;
        curr_pitch = robot_pitch;
        curr_roll = robot_roll;
        xSemaphoreGive(imuMutex);
    }

    // Format pengiriman: SENSOR:distance3,proxState,lightState,yaw,pitch,roll,prox2State,distance2,prox3State,distance,distance4
    Serial.print("SENSOR:");
    Serial.print(distance3); // bawah depan
    Serial.print(",");
    Serial.print(proxState); // capit
    Serial.print(",");
    Serial.print(lightState); 
    Serial.print(",");
    Serial.print(curr_yaw, 2); 
    Serial.print(",");
    Serial.print(curr_pitch, 2);
    Serial.print(",");
    Serial.print(curr_roll, 2);
    Serial.print(",");
    Serial.print(prox2State); //
    Serial.print(",");
    Serial.print(distance2); // kanan
    Serial.print(",");
    Serial.print(prox3State);  // [TAMBAHAN] Print status Proximity 3
    Serial.print(",");
    Serial.print(distance); // atas depan
    Serial.print(",");
    Serial.println(distance4);
    // Serial.print(",");
    // Serial.println(analogRead(PHOTO_PIN)); // [TAMBAHAN] Print jarak Ultrasonik 3
    
    // Pengiriman Serial setiap 20ms (50 Hz)
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// ======================================================
// SETUP & LOOP
// ======================================================
void setup() {
  Serial.begin(115200);
  
  // Inisialisasi I2C pada Clock Cepat (400kHz) direkomendasikan untuk DMP
  Wire.begin();
  Wire.setClock(400000); 
  
  // Inisialisasi Pin Ultrasonik 1
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Inisialisasi Pin Ultrasonik 2
  pinMode(TRIG2_PIN, OUTPUT);
  pinMode(ECHO2_PIN, INPUT);

  // [TAMBAHAN] Inisialisasi Pin Ultrasonik 3
  pinMode(TRIG3_PIN, OUTPUT);
  pinMode(ECHO3_PIN, INPUT);

  // Inisialisasi Pin Ultrasonik 4
  pinMode(TRIG4_PIN, OUTPUT);
  pinMode(ECHO4_PIN, INPUT);

  // Inisialisasi Pin Proximity
  pinMode(PROX_PIN, INPUT);
  pinMode(PROX2_PIN, INPUT); 
  pinMode(PROX3_PIN, INPUT); // [TAMBAHAN] Inisialisasi Pin Proximity 3
  
  analogReadResolution(12);

  // Inisialisasi Mutex
  imuMutex = xSemaphoreCreateMutex();
  
  // Membuat task sensor (Diletakkan di Core 0 agar pembacaan konsisten)
  xTaskCreatePinnedToCore(taskUltrasonic, "ReadUltrasonic", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskProximity,  "ReadProximity",  1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskPhotodiode, "ReadPhotodiode", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskMPU6050,    "ReadMPU6050",    4096, NULL, 3, NULL, 0); // Prioritas MPU ditingkatkan (3)
  
  // Membuat task printer/sender (Diletakkan di Core 1 agar komunikasi UART tidak putus-putus)
  xTaskCreatePinnedToCore(taskPrint, "SendToROS", 2048, NULL, 2, NULL, 1);
}

void loop() {
  // FreeRTOS menangani semua task, matikan fungsi loop standar
  vTaskDelete(NULL); 
}