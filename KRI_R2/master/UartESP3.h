#pragma once
#include <Arduino.h>

// 1. Struktur data untuk PERINTAH ke ESP ke-3
typedef struct {
    int commandID;   // Contoh: ID perintah (1 = capit buka, 2 = capit tutup, dll)
    float parameter; // Contoh: nilai sudut servo atau kecepatan
} ESP3_Message;

// 2. Struktur data untuk FEEDBACK dari ESP ke-3
typedef struct {
    int status;      // Contoh: status kerja (0 = idle, 1 = sibuk, 2 = error)
    float sensorA;   // Contoh: pembacaan sensor A di ESP3
    float sensorB;   // Contoh: pembacaan sensor B di ESP3
} ESP3_Feedback;

// Jadikan variabel struct bersifat global agar bisa diakses di file lain
extern ESP3_Message dataToESP3;
extern ESP3_Feedback feedbackFromESP3;

void setupESP3UART();
void sendToESP3(int cmd, float param);
void processESP3Data();