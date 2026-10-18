#include "Communication.h"
#include "MotorControl.h"
#include "Encoder.h"

uint8_t masterAddress[] = {0xD4, 0xE9, 0xF4, 0xA3, 0xA3, 0xE4};
struct_message incomingData;
struct_feedback feedbackToMaster;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingRawData, int len) {
  memcpy(&incomingData, incomingRawData, sizeof(incomingData));
  
  // Ubah setpoint ke objek Motor yang baru
  motFL.setpoint = incomingData.fl;
  motFR.setpoint = incomingData.fr;
  motRL.setpoint = incomingData.rl;
  motRR.setpoint = incomingData.rr;

  
}

void setupESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void sendFeedback() {
  feedbackToMaster.rpmFL = motFL.rpm; feedbackToMaster.rpmFR = motFR.rpm;
  feedbackToMaster.rpmRL = motRL.rpm; feedbackToMaster.rpmRR = motRR.rpm;
  
  // Hitungan total diambil dari objek Encoder
  feedbackToMaster.countFL = encFL.count; feedbackToMaster.countFR = encFR.count;
  feedbackToMaster.countRL = encRL.count; feedbackToMaster.countRR = encRR.count;

  esp_now_send(masterAddress, (uint8_t *) &feedbackToMaster, sizeof(feedbackToMaster));
}