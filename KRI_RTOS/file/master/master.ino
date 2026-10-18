#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Definisi Pin UART untuk Slave
#define RX2_PIN 16
#define TX2_PIN 17
#define RX1_PIN 25
#define TX1_PIN 26

// ======================================================
// STRUKTUR DATA (MASTER -> SLAVE)
// ======================================================
typedef struct __attribute__((packed)) struct_message_s1 { 
    int fl; int fr; int rl; int rr; 
} struct_message_s1;
struct_message_s1 motorToSlave1 = {0, 0, 0, 0};

typedef struct __attribute__((packed)) struct_message_s2 {
    float   data1; 
    bool    data2; 
    float   data3; 
    float   data4; 
    bool    data5; 
    bool    data6; 
    int     data7; 
    float   data8; 
    int     data9; 
} struct_message_s2;
struct_message_s2 motorToSlave2 = {0.0f, false, 0.0f, 0.0f, false, false, 0, 0.0f, 0};

// ======================================================
// STRUKTUR DATA FEEDBACK (SLAVE -> MASTER)
// ======================================================
typedef struct __attribute__((packed)) struct_feedback_s1 {
    float rpmRL;
    float rpmRR;
    float rpmFR;
} struct_feedback_s1;
struct_feedback_s1 feedback_s1 = {0.0, 0.0, 0.0};

typedef struct __attribute__((packed)) struct_feedback_s2 {
    float rpmFL;
} struct_feedback_s2;
struct_feedback_s2 feedback_s2 = {0.0};

// ======================================================
// TASK 1: Baca Serial ROS & Kirim ke Slave 1 & 2
// ======================================================
void TaskReceiveAndSend(void *pvParameters) {
    for (;;) {
        if (Serial.available() > 0) {
            String data = Serial.readStringUntil('\n');
            data.trim(); 

            float v[10] = {0}; 
            int idx = 0; int pos = 0;

            while ((pos = data.indexOf(',')) != -1 && idx < 9) {
                v[idx++] = data.substring(0, pos).toFloat();
                data = data.substring(pos + 1);
            }
            if (data.length() > 0 && idx < 10) {
                v[idx] = data.toFloat();
            }

            // Data untuk Slave 1
            motorToSlave1.fl = (int)v[0];
            motorToSlave1.fr = (int)v[1];
            motorToSlave1.rl = (int)v[2];
            motorToSlave1.rr = (int)v[3];

            // Data untuk Slave 2
            motorToSlave2.data1 = v[0];        
            motorToSlave2.data2 = (bool)v[6];  
            motorToSlave2.data3 = v[4];        
            motorToSlave2.data4 = v[7];        
            motorToSlave2.data5 = false;       
            motorToSlave2.data6 = false;       
            motorToSlave2.data7 = (int)v[5];   
            motorToSlave2.data8 = v[8];        
            motorToSlave2.data9 = (int)v[9];   
            
            // Kirim
            Serial2.write((uint8_t *)&motorToSlave1, sizeof(motorToSlave1));
            Serial1.write((uint8_t *)&motorToSlave2, sizeof(motorToSlave2));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ======================================================
// TASK 2: Baca Feedback dari Slave 1 & 2 -> Kirim ke ROS
// ======================================================
void TaskReadFeedback(void *pvParameters) {
    for (;;) {
        bool newData = false;

        // Baca feedback dari Slave 1
        if (Serial2.available() >= sizeof(struct_feedback_s1)) {
            Serial2.readBytes((uint8_t*)&feedback_s1, sizeof(struct_feedback_s1));
            newData = true;
            // Bersihkan sisa buffer agar tidak delay
            while(Serial2.available() > sizeof(struct_feedback_s1)) Serial2.read();
        }

        // Baca feedback dari Slave 2
        if (Serial1.available() >= sizeof(struct_feedback_s2)) {
            Serial1.readBytes((uint8_t*)&feedback_s2, sizeof(struct_feedback_s2));
            newData = true;
            // Bersihkan sisa buffer agar tidak delay
            while(Serial1.available() > sizeof(struct_feedback_s2)) Serial1.read();
        }

        // Jika ada data baru dari salah satu slave, print dengan format ROS: FEEDBACK:FL,FR,RL,RR
        if (newData) {
            Serial.printf("FEEDBACK:%.2f,%.2f,%.2f,%.2f\n", 
                          feedback_s2.rpmFL, 
                          feedback_s1.rpmFR, 
                          feedback_s1.rpmRL, 
                          feedback_s1.rpmRR);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Cek setiap 50ms
    }
}

// ======================================================
// SETUP 
// ======================================================
void setup() {
    delay(2000); 
    
    Serial.begin(115200);
    delay(500); 
    while(Serial.available() > 0) Serial.read();

    Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
    Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

    // Buat Task Kirim (Core 1)
    xTaskCreatePinnedToCore(TaskReceiveAndSend, "RcvAndSend", 4096, NULL, 1, NULL, 1); 
    // Buat Task Baca Feedback (Core 0)
    xTaskCreatePinnedToCore(TaskReadFeedback, "ReadFeedback", 4096, NULL, 1, NULL, 0); 
}

void loop() {
    vTaskDelete(NULL); 
}