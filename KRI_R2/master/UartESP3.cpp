#include "UartESP3.h"
#include "Config.h"

#define ESP3_BAUDRATE 115200

// Inisialisasi variabel struct di memori
ESP3_Message dataToESP3;
ESP3_Feedback feedbackFromESP3;

void setupESP3UART() {
    // Memulai Serial1 di ESP32 Master
    Serial1.begin(ESP3_BAUDRATE, SERIAL_8N1, ESP3_RX, ESP3_TX);
}

void sendToESP3(int cmd, float param) {
    // 1. Simpan data ke dalam struct
    dataToESP3.commandID = cmd;
    dataToESP3.parameter = param;

    // 2. Kirim isi struct via UART dengan format CSV: "cmd,param\n"
    Serial1.printf("%d,%.2f\n", dataToESP3.commandID, dataToESP3.parameter);
}

void processESP3Data() {
    // Cek apakah ada balasan dari ESP ke-3
    if (Serial1.available() > 0) {
        String incomingData = Serial1.readStringUntil('\n');
        
        // 1. Siapkan variabel sementara
        int tempStatus;
        float tempSensA, tempSensB;

        // 2. Parsing (pecah) format string balasan dari ESP3 (Format: "status,sensorA,sensorB\n")
        if (sscanf(incomingData.c_str(), "%d,%f,%f", &tempStatus, &tempSensA, &tempSensB) == 3) {
            
            // 3. Jika parsing sukses (dapat 3 data), masukkan ke dalam struct
            feedbackFromESP3.status = tempStatus;
            feedbackFromESP3.sensorA = tempSensA;
            feedbackFromESP3.sensorB = tempSensB;

            // Debugging (Opsional: lihat hasil bacaan di PC/ROS)
            // Serial.printf("INFO_ESP3: Status=%d, SensA=%.2f, SensB=%.2f\n", 
            //               feedbackFromESP3.status, 
            //               feedbackFromESP3.sensorA, 
            //               feedbackFromESP3.sensorB);
        }
    }
}