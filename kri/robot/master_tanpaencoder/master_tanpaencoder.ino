#include <esp_now.h>
#include <WiFi.h>

// PIN MOTOR LOCAL (DI MASTER)
#define FR_PWM   18
#define FR_IN1   4
#define FR_IN2   2

#define FL_PWM   19
#define FL_IN1   5
#define FL_IN2   23

// MAC Address Slave
uint8_t slaveAddress[] = {0x38, 0x18, 0x2B, 0x8A, 0xD1, 0x90}; 

typedef struct struct_message {
    int fl; int fr; int rl; int rr;
} struct_message;

// Struktur untuk Motor Lokal (Sederhana tanpa PID/Encoder)
struct Motor {
    int pwmPin; int in1; int in2;
    int pwmValue;
};

struct_message motorToSlave;
Motor TR = {FR_PWM, FR_IN1, FR_IN2, 0};
Motor TL = {FL_PWM, FL_IN1, FL_IN2, 0};

// Fungsi Direct Drive PWM
void putarMotor(Motor &m, int kecepatan) {
    int nilai_pwm = constrain(abs(kecepatan), 0, 255);
    if (kecepatan > 0) { 
        digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, nilai_pwm); 
    }
    else if (kecepatan < 0) { 
        digitalWrite(m.in1, LOW); digitalWrite(m.in2, HIGH); analogWrite(m.pwmPin, nilai_pwm); 
    }
    else { 
        digitalWrite(m.in1, LOW); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, 0); 
    }
}

void setup() {
    Serial.begin(115200);
    
    pinMode(TR.pwmPin, OUTPUT); pinMode(TR.in1, OUTPUT); pinMode(TR.in2, OUTPUT);
    pinMode(TL.pwmPin, OUTPUT); pinMode(TL.in1, OUTPUT); pinMode(TL.in2, OUTPUT);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return;
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, slaveAddress, 6);
    peerInfo.channel = 0; peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void loop() {
    // Terima data dari ROS: format "fl,fr,rl,rr,pwmTL,pwmTR"
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        float v[6]; int idx = 0; int pos = 0;
        
        while ((pos = data.indexOf(',')) != -1 && idx < 5) {
            v[idx++] = data.substring(0, pos).toFloat();
            data = data.substring(pos + 1);
        }
        v[idx] = data.toFloat();

        // Kirim ke Slave via ESP-NOW
        motorToSlave.fl = (int)v[0]; 
        motorToSlave.fr = (int)v[1];
        motorToSlave.rl = (int)v[2]; 
        motorToSlave.rr = (int)v[3];
        esp_now_send(slaveAddress, (uint8_t *) &motorToSlave, sizeof(motorToSlave));

        // Terapkan langsung ke motor lokal
        putarMotor(TL, (int)v[4]); 
        putarMotor(TR, (int)v[5]);
    }
}