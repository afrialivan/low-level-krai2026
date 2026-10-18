// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// // Port UART 2 -> Terhubung ke Slave 1
// #define RX2_PIN 16
// #define TX2_PIN 17

// // Port UART 1 -> Terhubung ke Slave 2 (Gunakan pin kosong bebas konflik)
// #define RX1_PIN 25
// #define TX1_PIN 26

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI SLAVE 1
// // ======================================================
// typedef struct struct_message_s1 {
//     int fl; int fr; int rl; int rr;
// } struct_message_s1;

// typedef struct struct_feedback_s1 {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback_s1;

// struct_message_s1 motorToSlave1;

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI SLAVE 2
// // ======================================================
// typedef struct struct_message_s2 {
//     float data1; float data2; float data3; float data4;
// } struct_message_s2;

// typedef struct struct_feedback_s2 {
//     float out1; float out2; float out3; float out4;
// } struct_feedback_s2;

// struct_message_s2 motorToSlave2;

// // Global buffer feedback penampung terakhir untuk diprint ke ROS
// struct_feedback_s1 fbS1_latest;
// struct_feedback_s2 fbS2_latest;

// SemaphoreHandle_t setpointMutex;  
// SemaphoreHandle_t feedbackMutex;

// // ======================================================
// // TASK 1: Baca Serial ROS & Kirim ke Slave 1 & Slave 2
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
//             if(idx == 9) v[idx] = data.toFloat();

//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 // Parsing Data untuk Slave 1
//                 motorToSlave1.fl = (int)v[0]; 
//                 motorToSlave1.fr = (int)v[1];
//                 motorToSlave1.rl = (int)v[2]; 
//                 motorToSlave1.rr = (int)v[3];
                
//                 // v[4] dan v[5] sebelumnya untuk motor Master, sekarang diabaikan
                
//                 // Parsing Data untuk Slave 2
//                 motorToSlave2.data1 = v[6];
//                 motorToSlave2.data2 = v[7];
//                 motorToSlave2.data3 = v[8];
//                 motorToSlave2.data4 = v[9];
//                 xSemaphoreGive(setpointMutex);
                
//                 // Kirim langsung ke masing-masing Slave lewat Jalur UART biner
//                 Serial2.write((uint8_t *)&motorToSlave1, sizeof(motorToSlave1));
//                 Serial1.write((uint8_t *)&motorToSlave2, sizeof(motorToSlave2));
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK 2: Terima Jalur UART dari Slave 1 & Slave 2
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//     struct_feedback_s1 s1Data;
//     struct_feedback_s2 s2Data;
//     for (;;) {
//         // Cek data masuk dari Slave 1
//         if (Serial2.available() >= sizeof(struct_feedback_s1)) {
//             Serial2.readBytes((uint8_t*)&s1Data, sizeof(struct_feedback_s1));
//             if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//                 fbS1_latest = s1Data;
//                 xSemaphoreGive(feedbackMutex);
//             }
//         }
//         // Cek data masuk dari Slave 2
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
// // TASK 3: Print Gabungan Feedback ke ROS Berdasarkan Timer (100ms)
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//             Serial.print("FEEDBACK:"); 
            
//             // Data RPM Slave 1
//             Serial.print(fbS1_latest.rpmFL);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmFR);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmRL);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmRR);   Serial.print(",");
            
//             // Data Encoder Count Slave 1
//             Serial.print(fbS1_latest.countFL); Serial.print(",");
//             Serial.print(fbS1_latest.countFR); Serial.print(",");
//             Serial.print(fbS1_latest.countRL); Serial.print(",");
//             Serial.print(fbS1_latest.countRR); Serial.print(","); 
            
//             // Data Output Slave 2
//             Serial.print(fbS2_latest.out1);     Serial.print(",");
//             Serial.print(fbS2_latest.out2);     Serial.print(",");
//             Serial.print(fbS2_latest.out3);     Serial.print(",");
//             Serial.println(fbS2_latest.out4);
            
//             xSemaphoreGive(feedbackMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// void setup() {
//     Serial.begin(115200);                               // Ke ROS PC
//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN); // Ke Slave 1
//     Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN); // Ke Slave 2
    
//     setpointMutex = xSemaphoreCreateMutex();
//     feedbackMutex = xSemaphoreCreateMutex();

//     xTaskCreatePinnedToCore(TaskSerialRead,  "SerRead",  4096, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRcv",  4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskSerialWrite, "SerWrite", 4096, NULL, 1, NULL, 0); 
// }

// void loop() {
//     vTaskDelete(NULL);
// }