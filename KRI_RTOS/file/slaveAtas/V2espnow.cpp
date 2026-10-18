// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <esp_now.h>
// #include <WiFi.h>

// // ======================================================
// // MAC ADDRESS MASTER (WAJIB DIGANTI DENGAN MAC ASLI MASTER)
// // ======================================================
// uint8_t macMaster[] = {0xD4, 0xE9, 0xF4, 0xA3, 0xA3, 0xE4}; // Ganti MAC Master

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI
// // ======================================================
// typedef struct struct_message {
//   float data1; float data2; float data3; float data4;
// } struct_message;

// typedef struct struct_feedback {
//   float out1; float out2; float out3; float out4;
// } struct_feedback;

// struct_message incomingData;
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
// // CALLBACK ESP-NOW: Menerima Data dari Master
// // ======================================================
// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
//     if (len == sizeof(struct_message)) {
//         if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
//             memcpy(&incomingData, incomingDataPtr, sizeof(struct_message));
//             RL.in1 = incomingData.data1;
//             RL.in2 = incomingData.data2;
//             RL.in3 = incomingData.data3;
//             RL.in4 = incomingData.data4;
//             xSemaphoreGive(dataMutex);
//         }
//     }
// }

// // ======================================================
// // TASK RTOS: Mengirim Feedback ke Master via ESP-NOW
// // ======================================================
// void TaskFeedbackSend(void *pvParameters) {
//   const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   for (;;) {
//     if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//       feedbackToMaster.out1 = RL.in1; 
//       feedbackToMaster.out2 = RL.in2;
//       feedbackToMaster.out3 = RL.in3; 
//       feedbackToMaster.out4 = RL.in4;
//       xSemaphoreGive(dataMutex);
//     }

//     // Kirim struct ke Master via ESP-NOW
//     esp_now_send(macMaster, (uint8_t *) &feedbackToMaster, sizeof(feedbackToMaster));

//     // Debugging via USB Serial (Opsional)
//     Serial.printf("Output 1: %.2f | 2: %.2f | 3: %.2f | 4: %.2f\n", RL.in1, RL.in2, RL.in3, RL.in4);

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// void setup() {
//   Serial.begin(115200);
  
//   // Setup Wi-Fi & ESP-NOW
//   WiFi.mode(WIFI_STA);
//   if (esp_now_init() != ESP_OK) {
//       Serial.println("Error initializing ESP-NOW");
//       return;
//   }

//   // Daftarkan callback penerima data
//   esp_now_register_recv_cb(OnDataRecv);

//   // Daftarkan Peer (Master)
//   esp_now_peer_info_t peerInfo = {};
//   peerInfo.channel = 0;  
//   peerInfo.encrypt = false;
//   memcpy(peerInfo.peer_addr, macMaster, 6);
//   esp_now_add_peer(&peerInfo);

//   dataMutex = xSemaphoreCreateMutex();

//   // Task UART Receive dihapus, cukup Task Kirim Data saja
//   xTaskCreatePinnedToCore(TaskFeedbackSend, "FBMaster", 4096, NULL, 1, NULL, 1);
// }

// void loop() {
//   vTaskDelete(NULL); 
// }