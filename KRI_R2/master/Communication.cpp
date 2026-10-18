#include "Communication.h"
#include "Config.h"
#include "MotorControl.h"
#include "IMU.h"
#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message { int fl; int fr; int rl; int rr; } struct_message;
typedef struct struct_feedback {
    float rpmFL; float rpmFR; float rpmRL; float rpmRR;
    long countFL; long countFR; long countRL; long countRR;
} struct_feedback;

struct_message motorToSlave;
struct_feedback feedbackData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&feedbackData, incomingData, sizeof(feedbackData));
    
    // Gabungkan data Slave + Motor Lokal + IMU lalu kirim ke ROS
    Serial.printf("FEEDBACK:%.2f,%.2f,%.2f,%.2f,%d,%d,%ld,%ld,%ld,%ld,0,0,%.2f\n",
                  feedbackData.rpmFL, feedbackData.rpmFR, feedbackData.rpmRL, feedbackData.rpmRR,
                  motTL.speed, motTR.speed, // Pengganti RPM lokal
                  feedbackData.countFL, feedbackData.countFR, feedbackData.countRL, feedbackData.countRR,
                  getYawIMU());
}

void setupESPNow() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return;
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, slaveAddress, 6);
    peerInfo.channel = 0; peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
    esp_now_register_recv_cb(OnDataRecv);
}

void sendToSlaveESPNow(int fl, int fr, int rl, int rr) {
    motorToSlave.fl = fl; motorToSlave.fr = fr;
    motorToSlave.rl = rl; motorToSlave.rr = rr;
    esp_now_send(slaveAddress, (uint8_t *) &motorToSlave, sizeof(motorToSlave));
}