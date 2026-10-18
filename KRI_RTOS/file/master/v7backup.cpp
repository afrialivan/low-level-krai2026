// #include <Arduino.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// // ======================================================
// // DEFINISI PIN UART
// // ======================================================
// #define RX2_PIN 16
// #define TX2_PIN 17
// #define RX1_PIN 25
// #define TX1_PIN 26

// // ======================================================
// // DEFINISI PIN INPUT
// // ======================================================
// #define BTN1_PIN 34
// #define BTN2_PIN 35
// #define BTN3_PIN -1 // Jika tidak digunakan, nanti di-bypass di pinMode
// #define BTN4_PIN 32

// // Pin Sensor Proximity
// #define PROXIMITY_PIN  4 // -1 berarti TIDAK DIGUNAKAN
// #define PROXIMITY2_PIN -1  

// // ======================================================
// // DEFINISI PIN MOTOR DRIVER
// // ======================================================
// #define MOTOR_PWM_PIN  22  // Kontrol Kecepatan (PWM)
// #define MOTOR_DIR1_PIN 19  // Kontrol Arah 1
// #define MOTOR_DIR2_PIN 18  // Kontrol Arah 2

// // #define MOTOR_RUN_TIME_MS 1500 

// // ======================================================
// // STRUKTUR DATA
// // ======================================================
// typedef struct struct_sensor_inputs {
//     uint8_t btn1;
//     uint8_t btn2;
//     uint8_t btn3;
//     uint8_t btn4;
//     uint8_t btn5;
//     uint8_t btn6;
//     uint8_t proximity;  
//     uint8_t proximity2; 
// } struct_sensor_inputs;

// struct_sensor_inputs globalInputs = {0, 0, 0, 0, 0, 0, 0, 0};

// // --- DATA SLAVE 1 & 2 ---
// typedef struct struct_message_s1 { int fl; int fr; int rl; int rr; } struct_message_s1;
// typedef struct __attribute__((packed)) struct_feedback_s1 { long countFR; long countRL; long countRR; } struct_feedback_s1;
// struct_message_s1 motorToSlave1 = {0, 0, 0, 0};

// typedef struct __attribute__((packed)) struct_message_s2 {
//     float data1; bool data2; float data3; float data4; bool data5; bool data6;
// } struct_message_s2;
// typedef struct struct_feedback_s2 { float out1; float out2; float out3; float out4; } struct_feedback_s2;
// struct_message_s2 motorToSlave2 = {0.0f, false, 0.0f, 0.0f, false, false}; 

// struct_feedback_s1 fbS1_latest;
// struct_feedback_s2 fbS2_latest;

// SemaphoreHandle_t setpointMutex;
// SemaphoreHandle_t feedbackMutex;

// // ======================================================
// // TASK 1: Baca Serial ROS
// // ======================================================
// void TaskSerialRead(void *pvParameters) {
//     for (;;) {
//         if (Serial.available() > 0) {
//             String data = Serial.readStringUntil('\n');
//             float v[10] = {0}; int idx = 0; int pos = 0;

//             while ((pos = data.indexOf(',')) != -1 && idx < 9) {
//                 v[idx++] = data.substring(0, pos).toFloat();
//                 data = data.substring(pos + 1);
//             }
//             if (idx == 9) v[idx] = data.toFloat();

//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 motorToSlave1.fl = (int)v[0];
//                 motorToSlave1.fr = (int)v[1];
//                 motorToSlave1.rl = (int)v[2];
//                 motorToSlave1.rr = (int)v[3];

//                 motorToSlave2.data1 = v[0];
//                 motorToSlave2.data2 = v[6]; 
//                 motorToSlave2.data3 = v[4];
//                 motorToSlave2.data4 = v[5];
                
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
// // TASK 4: Print Feedback ke ROS
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(100);
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//             Serial.print("FEEDBACK:");
//             Serial.print(fbS1_latest.countFR); Serial.print(","); 
//             Serial.print(fbS1_latest.countRL); Serial.print(","); 
//             Serial.print(fbS1_latest.countRR); Serial.print(",");
//             Serial.print(fbS2_latest.out1); Serial.print(",");
//             Serial.print(globalInputs.btn1); Serial.print(",");
//             Serial.print(globalInputs.btn2); Serial.print(",");
//             Serial.print(globalInputs.btn3); Serial.print(",");
//             Serial.print(globalInputs.btn4); Serial.print(",");
//             Serial.print(globalInputs.proximity); Serial.print(",");
//             Serial.println(globalInputs.proximity2); 
//             xSemaphoreGive(feedbackMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// // ======================================================
// // TASK 5: Baca Input Sensor dan Tombol (FIXED DARI PIN -1)
// // ======================================================
// void TaskReadInputs(void *pvParameters) {
//     for (;;) {
//         if (BTN1_PIN != -1) globalInputs.btn1 = !digitalRead(BTN1_PIN);
//         if (BTN2_PIN != -1) globalInputs.btn2 = !digitalRead(BTN2_PIN);
//         if (BTN3_PIN != -1) globalInputs.btn3 = !digitalRead(BTN3_PIN);
//         if (BTN4_PIN != -1) globalInputs.btn4 = !digitalRead(BTN4_PIN);
        
//         // Proteksi jika pin bernilai -1
//         if (PROXIMITY_PIN != -1) {
//             globalInputs.proximity = !digitalRead(PROXIMITY_PIN);
//         } else {
//             globalInputs.proximity = 0;
//         }

//         if (PROXIMITY2_PIN != -1) {
//             globalInputs.proximity2 = !digitalRead(PROXIMITY2_PIN); // 1 jika terdeteksi (Active Low)
//         } else {
//             globalInputs.proximity2 = 0;
//         }

//         vTaskDelay(pdMS_TO_TICKS(30)); 
//     }
// }

// // ======================================================
// // TASK 6: Kontrol Motor Berdasarkan Proximity 2 (DITINGKATKAN)
// // ======================================================
// void TaskActuatorMotor(void *pvParameters) {
//     digitalWrite(MOTOR_DIR1_PIN, LOW);
//     digitalWrite(MOTOR_DIR2_PIN, LOW);
//     analogWrite(MOTOR_PWM_PIN, 0);

//     for (;;) {
//         // Jika Proximity 2 Mendeteksi Objek
//         if (globalInputs.proximity2 == 1) {
//             // Serial.println("[ACTUATOR] Objek Terdeteksi! Memulai Pergerakan...");
//             vTaskDelay(pdMS_TO_TICKS(100)); // 400 untuk rpm tinggi
            
//             // 1. GERAK MAJU (Ubah PWM ke HIGH untuk tes jika analogWrite bermasalah)
//             digitalWrite(MOTOR_DIR1_PIN, HIGH);
//             digitalWrite(MOTOR_DIR2_PIN, LOW);
//             analogWrite(MOTOR_PWM_PIN, 200);

//             vTaskDelay(pdMS_TO_TICKS(3200)); 

//             // 2. STOP SEBENTAR 
//             analogWrite(MOTOR_PWM_PIN, 0);

//             vTaskDelay(pdMS_TO_TICKS(300));

//             // 3. GERAK KEMBALI (MUNDUR)
//             digitalWrite(MOTOR_DIR1_PIN, LOW);
//             digitalWrite(MOTOR_DIR2_PIN, HIGH);
//             analogWrite(MOTOR_PWM_PIN, 200);
//             vTaskDelay(pdMS_TO_TICKS(200)); 

//             // 4. STOP TOTAL
//             digitalWrite(MOTOR_DIR1_PIN, LOW);
//             digitalWrite(MOTOR_DIR2_PIN, LOW);
//             analogWrite(MOTOR_PWM_PIN, 0);

//             // Cooldown 2 detik agar tidak langsung berulang memicu motor jika objek belum pergi
//             vTaskDelay(pdMS_TO_TICKS(2000));
//         }
//         vTaskDelay(pdMS_TO_TICKS(50)); 
//     }
// }

// // ======================================================
// // SETUP
// // ======================================================
// void setup() {
//     Serial.begin(115200);
//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
//     Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//     while(Serial1.available() > 0) Serial1.read();
//     while(Serial2.available() > 0) Serial2.read();

//     // Setup Pin Tombol & Sensor dengan proteksi pin -1
//     if (BTN1_PIN != -1) pinMode(BTN1_PIN, INPUT_PULLUP);
//     if (BTN2_PIN != -1) pinMode(BTN2_PIN, INPUT_PULLUP);
//     if (BTN3_PIN != -1) pinMode(BTN3_PIN, INPUT_PULLUP);
//     if (BTN4_PIN != -1) pinMode(BTN4_PIN, INPUT_PULLUP);
//     if (PROXIMITY_PIN != -1) pinMode(PROXIMITY_PIN, INPUT_PULLUP);
//     if (PROXIMITY2_PIN != -1) pinMode(PROXIMITY2_PIN, INPUT_PULLUP); 

//     // Setup Pin Motor Driver
//     pinMode(MOTOR_PWM_PIN, OUTPUT);
//     pinMode(MOTOR_DIR1_PIN, OUTPUT);
//     pinMode(MOTOR_DIR2_PIN, OUTPUT);

//     setpointMutex = xSemaphoreCreateMutex();
//     feedbackMutex = xSemaphoreCreateMutex();

//     // Create Tasks
//     xTaskCreatePinnedToCore(TaskSerialRead,    "SerRead",   4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskUARTReceive,  "UARTRcv",   4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskSerialWrite,  "SerWrite",  4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskReadInputs,   "ReadInp",   2048, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskSendSlave2,   "SendS2",    2048, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskActuatorMotor, "Actuator",  2048, NULL, 1, NULL, 1); 
// }

// void loop() {
//     vTaskDelete(NULL); 
// }