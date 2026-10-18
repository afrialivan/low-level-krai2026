// #include <Arduino.h>
// #include <Wire.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// #include "I2Cdev.h"
// #include "MPU6050_6Axis_MotionApps20.h"

// #define RX2_PIN 16
// #define TX2_PIN 17
// #define RX1_PIN 25
// #define TX1_PIN 26

// #define I2C_SDA 21
// #define I2C_SCL 22 

// #define BTN1_PIN 34
// #define BTN2_PIN 35
// #define BTN3_PIN -1 
// #define BTN4_PIN 32

// #define PROXIMITY_PIN  4 
// #define PROXIMITY2_PIN -1  

// #define MOTOR_PWM_PIN  23
// #define MOTOR_DIR1_PIN 19
// #define MOTOR_DIR2_PIN 18

// MPU6050 mpu;
// bool dmpReady = false;  
// uint8_t devStatus;      
// uint16_t packetSize;    
// uint8_t fifoBuffer[64]; 

// Quaternion q;           // [w, x, y, z]         Vektor Kuaternion
// VectorFloat gravity;    // [x, y, z]            Vektor Gravitasi
// float ypr[3];           // [yaw, pitch, roll]   Kontainer Yaw/Pitch/Roll

// float robot_yaw = 0.0f; // Variabel global untuk menyimpan sudut Yaw

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI
// // ======================================================
// typedef struct struct_sensor_inputs {
//     uint8_t btn1; uint8_t btn2; uint8_t btn3; uint8_t btn4;
//     uint8_t btn5; uint8_t btn6; uint8_t proximity; uint8_t proximity2; 
// } struct_sensor_inputs;

// struct_sensor_inputs globalInputs = {0, 0, 0, 0, 0, 0, 0, 0};

// typedef struct struct_message_s1 { int fl; int fr; int rl; int rr; } struct_message_s1;
// typedef struct __attribute__((packed)) struct_feedback_s1 { 
//     long countFR; 
//     long countRL; 
//     long countRR; 
// } struct_feedback_s1;
// struct_message_s1 motorToSlave1 = {0, 0, 0, 0};

// typedef struct __attribute__((packed)) struct_message_s2 {
//     float   data1; 
//     bool    data2; 
//     float   data3; 
//     float   data4; 
//     bool    data5; 
//     bool    data6; 
//     int     data7; 
//     float   data8;  // TAMBAHAN DUMMY
//     int     data9;  // TAMBAHAN DUMMY
// } struct_message_s2;

// struct_message_s2 motorToSlave2 = {0.0f, false, 0.0f, 0.0f, false, false, 0, 0.0f, 0};

// typedef struct struct_feedback_s2 { float out1; float out2; float out3; float out4; } struct_feedback_s2;

// struct_feedback_s1 fbS1_latest;
// struct_feedback_s2 fbS2_latest;

// // Mutex FreeRTOS
// SemaphoreHandle_t setpointMutex;
// SemaphoreHandle_t feedbackMutex;
// SemaphoreHandle_t imuMutex;

// void TaskIMURead(void *pvParameters) {
//     mpu.initialize();
    
//     if(!mpu.testConnection()) {
//         Serial.println("MPU6050 koneksi gagal. Task IMU dihentikan.");
//         vTaskDelete(NULL);
//     }

//     devStatus = mpu.dmpInitialize();

//     mpu.setXAccelOffset(0);
//     mpu.setYAccelOffset(0);
//     mpu.setZAccelOffset(0);
//     mpu.setXGyroOffset(0);
//     mpu.setYGyroOffset(0);
//     mpu.setZGyroOffset(0);

//     if (devStatus == 0) {
//         mpu.CalibrateAccel(6);
//         mpu.CalibrateGyro(6);
//         mpu.setDMPEnabled(true);
//         dmpReady = true;
//         packetSize = mpu.dmpGetFIFOPacketSize();
//     } else {
//         Serial.print("DMP Error: "); Serial.println(devStatus);
//         vTaskDelete(NULL);
//     }

//     for (;;) {
//         if (!dmpReady) {
//             vTaskDelay(pdMS_TO_TICKS(10));
//             continue;
//         }

//         // Baca FIFO Buffer DMP terbaru
//         if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {  
//             mpu.dmpGetQuaternion(&q, fifoBuffer);
//             mpu.dmpGetGravity(&gravity, &q);
//             mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

//             if (xSemaphoreTake(imuMutex, portMAX_DELAY) == pdTRUE) {
//                 // ypr[0] adalah Yaw dalam radian. Dikonversi ke derajat.
//                 robot_yaw = ypr[0] * 180.0 / M_PI;
//                 xSemaphoreGive(imuMutex);
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10)); // Eksekusi dengan interval ~10ms (100Hz)
//     }
// }

// // ======================================================
// // TASK 1: Baca Serial ROS
// // ======================================================
// // UBAH BENTUK PARSING DI MASTER ESP32 (TaskSerialRead)
// void TaskSerialRead(void *pvParameters) {
//     for (;;) {
//         if (Serial.available() > 0) {
//             String data = Serial.readStringUntil('\n');
            
//             // TAMBAHAN PENTING: Bersihkan string dari karakter \r
//             data.trim(); 

//             float v[11] = {0}; int idx = 0; int pos = 0;

//             while ((pos = data.indexOf(',')) != -1 && idx < 10) {
//                 v[idx++] = data.substring(0, pos).toFloat();
//                 data = data.substring(pos + 1);
//             }
            
//             if (data.length() > 0 && idx < 11) {
//                 v[idx] = data.toFloat();
//             }

//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 motorToSlave1.fr = (int)v[0];
//                 motorToSlave1.rl = (int)v[1];
//                 motorToSlave1.fl = (int)v[2];
//                 motorToSlave1.rr = (int)v[3];

//                 motorToSlave2.data1 = v[2];
//                 motorToSlave2.data2 = v[6]; 
//                 motorToSlave2.data3 = v[4]; // climb
//                 motorToSlave2.data4 = v[5]; // cahaya
//                 motorToSlave2.data7 = (int)v[7]; // motor climb
//                 motorToSlave2.data8 = v[8]; // Dummy 8
//                 motorToSlave2.data9 = (int)v[9]; // Dummy 9
                
//                 xSemaphoreGive(setpointMutex);
//                 Serial2.write((uint8_t *)&motorToSlave1, sizeof(motorToSlave1));
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK 2: Kirim Data ke Slave 2
// // ======================================================
// void TaskSendSlave2(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(50); 
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//             motorToSlave2.data5 = (bool)globalInputs.btn1; 
//             motorToSlave2.data6 = (bool)globalInputs.btn2; 
        
//             Serial1.write((uint8_t *)&motorToSlave2, sizeof(motorToSlave2));
//             xSemaphoreGive(setpointMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// // ======================================================
// // TASK 3: Terima UART dari Slave 1 & Slave 2
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//     struct_feedback_s1 s1Data;
//     struct_feedback_s2 s2Data;
//     for (;;) {
//         if (Serial2.available() >= sizeof(struct_feedback_s1)) {
//             Serial2.readBytes((uint8_t*)&s1Data, sizeof(struct_feedback_s1));
//             if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//                 fbS1_latest = s1Data;
//                 xSemaphoreGive(feedbackMutex);
//             }
//         }
//         if (Serial1.available() >= sizeof(struct_feedback_s2)) {
//             Serial1.readBytes((uint8_t*)&s2Data, sizeof(struct_feedback_s2));
//             if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//                 fbS2_latest = s2Data;
//                 xSemaphoreGive(feedbackMutex);
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK 4: Print Feedback ke ROS (Ditambahkan Yaw)
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     // const TickType_t xFrequency = pdMS_TO_TICKS(100);
//     const TickType_t xFrequency = pdMS_TO_TICKS(20);

//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         float current_yaw = 0.0f;
//         if (xSemaphoreTake(imuMutex, portMAX_DELAY) == pdTRUE) {
//             current_yaw = robot_yaw;
//             xSemaphoreGive(imuMutex);
//         }

//         if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//             Serial.print("FEEDBACK:");
//             Serial.print(fbS1_latest.countFR); Serial.print(","); //motor kanan depan
//             Serial.print(fbS2_latest.out1); Serial.print(","); // //motor kiri depan
//             Serial.print(fbS1_latest.countRR); Serial.print(","); //motor kanan belakang
//             Serial.print(fbS1_latest.countRL); Serial.print(","); //motor kiri belakang
//             Serial.print(globalInputs.btn1); Serial.print(",");
//             Serial.print(globalInputs.btn2); Serial.print(",");
//             Serial.print(globalInputs.btn3); Serial.print(",");
//             Serial.print(globalInputs.btn4); Serial.print(",");
//             Serial.print(globalInputs.proximity); Serial.print(",");
//             Serial.print(globalInputs.proximity2); Serial.print(","); 
//             Serial.println(current_yaw); // Data Yaw diletakkan di indeks terakhir (ke-10 di Python)
//             xSemaphoreGive(feedbackMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// // ======================================================
// // TASK 5: Baca Input Sensor dan Tombol
// // ======================================================
// void TaskReadInputs(void *pvParameters) {
//     for (;;) {
//         if (BTN1_PIN != -1) globalInputs.btn1 = !digitalRead(BTN1_PIN);
//         if (BTN2_PIN != -1) globalInputs.btn2 = !digitalRead(BTN2_PIN);
//         if (BTN3_PIN != -1) globalInputs.btn3 = !digitalRead(BTN3_PIN);
//         if (BTN4_PIN != -1) globalInputs.btn4 = !digitalRead(BTN4_PIN);
        
//         if (PROXIMITY_PIN != -1) globalInputs.proximity = !digitalRead(PROXIMITY_PIN);
//         else globalInputs.proximity = 0;

//         if (PROXIMITY2_PIN != -1) globalInputs.proximity2 = !digitalRead(PROXIMITY2_PIN); 
//         else globalInputs.proximity2 = 0;

//         vTaskDelay(pdMS_TO_TICKS(30)); 
//     }
// }

// // ======================================================
// // TASK 6: Kontrol Motor Berdasarkan Proximity 2
// // ======================================================
// void TaskActuatorMotor(void *pvParameters) {
//     digitalWrite(MOTOR_DIR1_PIN, LOW);
//     digitalWrite(MOTOR_DIR2_PIN, LOW);
//     analogWrite(MOTOR_PWM_PIN, 0);

//     for (;;) {
//         if (globalInputs.proximity2 == 1) {
//             vTaskDelay(pdMS_TO_TICKS(100)); 
//             digitalWrite(MOTOR_DIR1_PIN, HIGH);
//             digitalWrite(MOTOR_DIR2_PIN, LOW);
//             analogWrite(MOTOR_PWM_PIN, 200);
//             vTaskDelay(pdMS_TO_TICKS(3200)); 
            
//             analogWrite(MOTOR_PWM_PIN, 0);
//             vTaskDelay(pdMS_TO_TICKS(300));
            
//             digitalWrite(MOTOR_DIR1_PIN, LOW);
//             digitalWrite(MOTOR_DIR2_PIN, HIGH);
//             analogWrite(MOTOR_PWM_PIN, 200);
//             vTaskDelay(pdMS_TO_TICKS(200)); 
            
//             digitalWrite(MOTOR_DIR1_PIN, LOW);
//             digitalWrite(MOTOR_DIR2_PIN, LOW);
//             analogWrite(MOTOR_PWM_PIN, 0);
//             vTaskDelay(pdMS_TO_TICKS(2000));
//         }
//         vTaskDelay(pdMS_TO_TICKS(50)); 
//     }
// }

// // ======================================================
// // SETUP 
// // ======================================================
// void setup() {
//     delay(2000); 
//     Serial.begin(115200);
//     delay(500); 
//     while(Serial.available() > 0) Serial.read();

//     // Inisialisasi I2C (Pin 21 SDA, Pin 22 SCL) pada Clock Cepat 400kHz
//     Wire.begin(I2C_SDA, I2C_SCL);
//     Wire.setClock(400000); 

//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
//     Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//     while(Serial1.available() > 0) Serial1.read();
//     while(Serial2.available() > 0) Serial2.read();

//     if (BTN1_PIN != -1) pinMode(BTN1_PIN, INPUT_PULLUP);
//     if (BTN2_PIN != -1) pinMode(BTN2_PIN, INPUT_PULLUP);
//     if (BTN3_PIN != -1) pinMode(BTN3_PIN, INPUT_PULLUP);
//     if (BTN4_PIN != -1) pinMode(BTN4_PIN, INPUT_PULLUP);
//     if (PROXIMITY_PIN != -1) pinMode(PROXIMITY_PIN, INPUT_PULLUP);
//     if (PROXIMITY2_PIN != -1) pinMode(PROXIMITY2_PIN, INPUT_PULLUP); 

//     pinMode(MOTOR_PWM_PIN, OUTPUT);
//     pinMode(MOTOR_DIR1_PIN, OUTPUT);
//     pinMode(MOTOR_DIR2_PIN, OUTPUT);

//     setpointMutex = xSemaphoreCreateMutex();
//     feedbackMutex = xSemaphoreCreateMutex();
//     imuMutex = xSemaphoreCreateMutex();

//     // Create RTOS Tasks
//     // Task IMU di Core 0, Task lain di Core 1 agar MPU6050 tidak terganggu komunikasi Serial
//     xTaskCreatePinnedToCore(TaskIMURead,       "IMURead",   4096, NULL, 3, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskSerialRead,    "SerRead",   4096, NULL, 1, NULL, 1);
//     xTaskCreatePinnedToCore(TaskUARTReceive,  "UARTRcv",   4096, NULL, 1, NULL, 1);
//     xTaskCreatePinnedToCore(TaskSerialWrite,  "SerWrite",  4096, NULL, 1, NULL, 1);
//     xTaskCreatePinnedToCore(TaskReadInputs,   "ReadInp",   2048, NULL, 1, NULL, 1); 
//     xTaskCreatePinnedToCore(TaskSendSlave2,   "SendS2",    2048, NULL, 1, NULL, 1); 
//     xTaskCreatePinnedToCore(TaskActuatorMotor, "Actuator",  2048, NULL, 1, NULL, 1); 
// }

// void loop() {
//     vTaskDelete(NULL); 
// }