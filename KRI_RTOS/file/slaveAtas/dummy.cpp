// #include <esp_now.h>
// #include <WiFi.h>

// // ======================================================
// // STRUKTUR DATA ESP-NOW
// // ======================================================
// uint8_t masterAddress[] = {0xD4, 0xE9, 0xF4, 0xA3, 0xA3, 0xE4};


// typedef struct struct_message {
//   float data1;
//   float data2;
//   float data3;
//   float data4;
// } struct_message;

// struct_message incomingData;

// typedef struct struct_feedback {
//     float out1; 
//     float out2; 
//     float out3; 
//     float out4;
// } struct_feedback;

// struct_feedback feedbackToMaster;

// // ======================================================
// // STRUKTUR MOTOR
// // ======================================================
// struct Motor {
//   float in1;
//   float in2;
//   float in3;
//   float in4;
// };

// // Inisialisasi Objek Motor
// Motor RL = { 0, 0, 0, 0 };


// void OnDataRecv(const uint8_t *mac, const uint8_t *incomingRawData, int len) {
//   memcpy(&incomingData, incomingRawData, sizeof(incomingData));
//   RL.in1 = incomingData.data1;
//   RL.in2 = incomingData.data2;
//   RL.in3 = incomingData.data3;
//   RL.in4 = incomingData.data4;
// }

// unsigned long lastTime = 0;
// float dt = 0.1;

// void setup() {
//   Serial.begin(115200);

//   // WAJIB: Inisialisasi WiFi dan ESP-NOW
//   WiFi.mode(WIFI_STA);
//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error ESP-NOW Init");
//     return;
//   }

//   esp_now_register_recv_cb(OnDataRecv);

//   // Daftarkan Master sebagai Peer
//   esp_now_peer_info_t peerInfo = {};
//   memcpy(peerInfo.peer_addr, masterAddress, 6);
//   peerInfo.channel = 0;
//   peerInfo.encrypt = false;
  
//   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//     Serial.println("Gagal tambah Peer");
//     return;
//   }

//   lastTime = millis();
// }

// void loop() {
//   unsigned long now = millis();

//   if (now - lastTime >= 100) {

//     // Siapkan Feedback
//     feedbackToMaster.out1 = RL.in1; 
//     feedbackToMaster.out2 = RL.in2;
//     feedbackToMaster.out3 = RL.in3; 
//     feedbackToMaster.out4 = RL.in4;

//     // Kirim ke Master
//     esp_now_send(masterAddress, (uint8_t *) &feedbackToMaster, sizeof(feedbackToMaster));

//     // Debugging (pindahkan ke sini agar tidak membebani PID)
//     Serial.printf("RPM FR: %.2f | FL: %.2f\n | RR: %.2f | RL: %.2f\n", RL.in1, RL.in2, RL.in3, RL.in4);

//     lastTime = now;
//   }
// }