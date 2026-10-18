// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>

// // PIN UART untuk Slave 2 (Sesuaikan dengan pin kosong di ESP32 Anda)
// #define RX1_PIN 25
// #define TX1_PIN 26

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI (Wajib Sinkron dengan Master)
// // ======================================================
// typedef struct struct_message {
//   float data1;
//   float data2;
//   float data3;
//   float data4;
// } struct_message;

// struct_message incomingData;

// typedef struct struct_feedback {
//   float out1; 
//   float out2; 
//   float out3; 
//   float out4;
// } struct_feedback;

// struct_feedback feedbackToMaster;

// // ======================================================
// // STRUKTUR MOTOR
// // ======================================================
// struct Motor {
//   float in1; float in2; float in3; float in4;
// };

// Motor RL = { 0, 0, 0, 0 };
// SemaphoreHandle_t dataMutex;

// // ======================================================
// // TASK RTOS 1: Menerima Data Setpoint dari Master via UART
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//   for (;;) {
//     if (Serial1.available() >= sizeof(struct_message)) {
//       struct_message tempMsg;
//       Serial1.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

//       if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//         incomingData = tempMsg;
//         RL.in1 = incomingData.data1;
//         RL.in2 = incomingData.data2;
//         RL.in3 = incomingData.data3;
//         RL.in4 = incomingData.data4;
//         xSemaphoreGive(dataMutex);
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// }

// // ======================================================
// // TASK RTOS 2: Mengirim Feedback ke Master via UART
// // ======================================================
// void TaskFeedbackSend(void *pvParameters) {
//   const TickType_t xFrequency = pdMS_TO_TICKS(100); // Kirim berkala tiap 100ms
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   for (;;) {
//     if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//       feedbackToMaster.out1 = RL.in1; 
//       feedbackToMaster.out2 = RL.in2;
//       feedbackToMaster.out3 = RL.in3; 
//       feedbackToMaster.out4 = RL.in4;
//       xSemaphoreGive(dataMutex);
//     }

//     // Kirim struct biner ke Master
//     Serial1.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));

//     // Debugging via Serial Monitor USB
//     Serial.printf("RPM FR: %.2f | FL: %.2f | RR: %.2f | RL: %.2f\n", RL.in1, RL.in2, RL.in3, RL.in4);

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// void setup() {
//   Serial.begin(115200);
  
//   // Menggunakan Serial1 untuk komunikasi antar-mikrokontroler
//   Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//   dataMutex = xSemaphoreCreateMutex();

//   // Task UART Receive di Core 0
//   xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRecv", 2048, NULL, 1, NULL, 0);

//   // Task Kirim Data di Core 1
//   xTaskCreatePinnedToCore(TaskFeedbackSend, "FBMaster", 2048, NULL, 1, NULL, 1);
// }

// void loop() {
//   vTaskDelete(NULL); 
// }