// #include <Arduino.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// // ======================================================
// // DEFINISI PIN UART
// // ======================================================
// // Port UART 2 -> Terhubung ke Slave 1 (Motor Controller)
// #define RX2_PIN 16
// #define TX2_PIN 17

// // Port UART 1 -> Terhubung ke Slave 2 (Mekanisme Tambahan/Aktuator)
// #define RX1_PIN 25
// #define TX1_PIN 26

// // ======================================================
// // DEFINISI PIN INPUT
// // ======================================================
// #define BTN1_PIN 34
// #define BTN2_PIN 35
// #define BTN3_PIN 25 // Catatan: Pin 25 juga dipakai RX1_PIN di atas. Pastikan tidak bentrok di hardware asli!
// #define BTN4_PIN 32
// // #define BTN5_PIN 26 // Catatan: Pin 26 juga dipakai TX1_PIN di atas.
// // #define BTN6_PIN 33

// // Pin Sensor E18-D80NK
// #define PROXIMITY_PIN 4

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
//     uint8_t proximity; // 1 = Objek terdeteksi, 0 = Tidak ada objek
// } struct_sensor_inputs;

// struct_sensor_inputs globalInputs = {0, 0, 0, 0, 0, 0, 0};

// // --- DATA SLAVE 1 ---
// typedef struct struct_message_s1 {
//     int fl; int fr; int rl; int rr;
// } struct_message_s1;

// typedef struct __attribute__((packed)) struct_feedback_s1 {
//     long countFR; long countRL; long countRR;
// } struct_feedback_s1;

// struct_message_s1 motorToSlave1 = {0, 0, 0, 0};

// // --- DATA SLAVE 2 ---
// typedef struct __attribute__((packed)) struct_message_s2 {
//     float   data1; 
//     bool    data2; 
//     float   data3; 
//     float   data4; 
//     bool    data5;
//     bool    data6;
// } struct_message_s2;

// typedef struct struct_feedback_s2 {
//     float out1; float out2; float out3; float out4;
// } struct_feedback_s2;

// // Perbaikan: Inisialisasi 5 elemen sesuai isi struct
// struct_message_s2 motorToSlave2 = {0.0f, false, 0.0f, 0.0f, false, false}; 

// struct_feedback_s1 fbS1_latest;
// struct_feedback_s2 fbS2_latest;

// SemaphoreHandle_t setpointMutex;
// SemaphoreHandle_t feedbackMutex;

// // ======================================================
// // TASK 1: Baca Serial ROS (Hanya Update Data, Kirim ke S1)
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
//                 // Update data untuk Slave 1
//                 motorToSlave1.fl = (int)v[0];
//                 motorToSlave1.fr = (int)v[1];
//                 motorToSlave1.rl = (int)v[2];
//                 motorToSlave1.rr = (int)v[3];

//                 // Update data untuk Slave 2 (hanya data dari ROS)
//                 motorToSlave2.data1 = v[0];
//                 motorToSlave2.data2 = v[6]; 
//                 motorToSlave2.data3 = v[4];
//                 motorToSlave2.data4 = v[5];
                
//                 xSemaphoreGive(setpointMutex);

//                 // Kirim ke Slave 1 setiap kali ROS mengirim data
//                 Serial2.write((uint8_t *)&motorToSlave1, sizeof(motorToSlave1));
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK BARU: Kirim Data ke Slave 2 Secara Periodik (Real-Time)
// // ======================================================
// void TaskSendSlave2(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(50); // Kirim setiap 50ms
//     TickType_t xLastWakeTime = xTaskGetTickCount();
    
//     for (;;) {
//         if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//             // Update data tombol tepat sebelum dikirim
//             motorToSlave2.data5 = (bool)globalInputs.btn1; 
//             motorToSlave2.data6 = (bool)globalInputs.btn2; 
            
//             // Kirim ke Slave 2 terlepas dari ada/tidaknya data ROS
//             Serial1.write((uint8_t *)&motorToSlave2, sizeof(motorToSlave2));
            
//             xSemaphoreGive(setpointMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// // ======================================================
// // TASK 2: Terima UART dari Slave 1 & Slave 2
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
// // TASK 3: Print Feedback ke ROS (100ms)
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
//             Serial.println(globalInputs.proximity);
//             xSemaphoreGive(feedbackMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// // ======================================================
// // TASK 4: Baca Input Sensor dan Tombol
// // ======================================================
// void TaskReadInputs(void *pvParameters) {
//     for (;;) {
//         globalInputs.btn1 = !digitalRead(BTN1_PIN);
//         globalInputs.btn2 = !digitalRead(BTN2_PIN);
//         globalInputs.btn3 = !digitalRead(BTN3_PIN);
//         globalInputs.btn4 = !digitalRead(BTN4_PIN);
//         globalInputs.proximity = !digitalRead(PROXIMITY_PIN);

//         vTaskDelay(pdMS_TO_TICKS(30)); // Debounce & efisiensi
//     }
// }

// // ======================================================
// // SETUP
// // ======================================================
// void setup() {
//     Serial.begin(115200);
//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
//     Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//     // Bersihkan buffer serial Master
//     while(Serial1.available() > 0) Serial1.read();
//     while(Serial2.available() > 0) Serial2.read();

//     pinMode(BTN1_PIN, INPUT_PULLUP);
//     pinMode(BTN2_PIN, INPUT_PULLUP);
//     pinMode(BTN3_PIN, INPUT_PULLUP);
//     pinMode(BTN4_PIN, INPUT_PULLUP);
//     pinMode(PROXIMITY_PIN, INPUT_PULLUP);

//     setpointMutex = xSemaphoreCreateMutex();
//     feedbackMutex = xSemaphoreCreateMutex();

//     // ======================================================
//     // DAFTAR TASK FREERTOS
//     // ======================================================
//     xTaskCreatePinnedToCore(TaskSerialRead,  "SerRead",   4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRcv",   4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskSerialWrite, "SerWrite",  4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskReadInputs,  "ReadInp",   2048, NULL, 1, NULL, 0); 
    
//     // Task baru untuk Slave 2
//     xTaskCreatePinnedToCore(TaskSendSlave2,  "SendS2",    2048, NULL, 1, NULL, 0); 
// }

// // ======================================================
// // LOOP
// // ======================================================
// void loop() {
//     vTaskDelete(NULL); // Kosongkan loop utama agar FreeRTOS bekerja sepenuhnya
// }