// #include <esp_now.h>
// #include <WiFi.h>
// // #include <Wire.h>          
// // #include "MPU9250.h" // Library Hideaki Tai

// // Inisialisasi Sensor
// // MPU9250 mpu;

// // ======================================================
// // PIN MOTOR LOCAL (DI MASTER)
// // ======================================================
// //left
// #define FR_PWM   18
// #define FR_IN1   4
// #define FR_IN2   2
// #define FR_ENC_A 34
// #define FR_ENC_B 35

// //right
// #define FL_PWM   19
// #define FL_IN1   5
// #define FL_IN2   23
// #define FL_ENC_A 32
// #define FL_ENC_B 33

// // ======================================================
// // MAC ADDRESS
// // ======================================================
// // MAC Address Slave Pertama
// uint8_t slaveAddress[] = {0x20, 0xE7, 0xC8, 0xA7, 0x9B, 0x0C}; 

// // MAC Address ESP ke-3 (Ganti dengan MAC Address yang benar!)
// uint8_t esp3Address[] = {0x38, 0x18, 0x2B, 0x8A, 0xD1, 0x90}; 

// // ======================================================
// // STRUKTUR DATA ESP-NOW
// // ======================================================
// // Struktur data kirim ke Slave Pertama (Data 1-6 diperlukan untuk motor)
// typedef struct struct_message {
//     int fl; int fr; int rl; int rr;
// } struct_message;

// // Struktur feedback dari Slave Pertama
// typedef struct struct_feedback {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback;

// // Struktur data khusus data ke 6-9 untuk ESP ke-3
// typedef struct struct_to_esp3 {
//     float data1;
//     float data2;
//     float data3;
//     float data4;
// } struct_to_esp3;

// struct_message motorToSlave;
// struct_feedback feedbackData;
// struct_to_esp3 dataKeEsp3;

// // ======================================================
// // STRUKTUR & INISIALISASI MOTOR LOKAL
// // ======================================================
// struct Motor {
//     int pwmPin; int in1; int in2; int encA; int encB;
//     volatile long encoderCount;
//     volatile int lastEncoded;
//     long lastCount;
//     float ppr; float setpoint; float rpm;
//     float Kp, Ki, Kd, error, lastError, integral, derivative, control;
//     int pwmOutput;
// };

// // Inisialisasi Motor Lokal (Roda Depan/Top)
// Motor TR = {FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 1001, 0, 0, 1.0, 3.5, 0.03, 0, 0, 0, 0, 0, 0};
// Motor TL = {FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1001, 0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0};

// // ISR untuk Encoder Lokal
// void IRAM_ATTR encoderFR() { updateEncoder(TR); }
// void IRAM_ATTR encoderFL() { updateEncoder(TL); }

// void updateEncoder(Motor &m) {
//     int A = digitalRead(m.encA);
//     int B = digitalRead(m.encB);
//     int encoded = (A << 1) | B;
//     int sum = (m.lastEncoded << 2) | encoded;
//     if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) m.encoderCount++;
//     if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) m.encoderCount--;
//     m.lastEncoded = encoded;
// }

// void putarMotor(Motor &m, int kecepatan) {
//     int nilai_pwm = constrain(abs(kecepatan), 0, 255);
//     if (kecepatan > 0) { digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, nilai_pwm); }
//     else if (kecepatan < 0) { digitalWrite(m.in1, LOW); digitalWrite(m.in2, HIGH); analogWrite(m.pwmPin, nilai_pwm); }
//     else { digitalWrite(m.in1, LOW); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, 0); }
// }

// void updateMotorPID(Motor &m, float dt) {
//     long delta = m.encoderCount - m.lastCount;
//     m.lastCount = m.encoderCount;
//     m.rpm = (delta * 600.0) / m.ppr; 
//     m.error = abs(m.setpoint) - abs(m.rpm);
//     m.integral = constrain(m.integral + (m.error * dt), -500, 500);
//     m.derivative = (m.error - m.lastError) / dt;
//     m.control = (m.Kp * m.error) + (m.Ki * m.integral) + (m.Kd * m.derivative);
//     m.pwmOutput = constrain(abs(m.control), 0, 255);
//     if (m.setpoint > 0) putarMotor(m, m.pwmOutput);
//     else if (m.setpoint < 0) putarMotor(m, -m.pwmOutput);
//     else { putarMotor(m, 0); m.integral = 0; }
//     m.lastError = m.error;
// }

// // Callback saat terima data dari Slave
// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//     memcpy(&feedbackData, incomingData, sizeof(feedbackData));

//     // Kirim Feedback Gabungan ke ROS via Serial
//     Serial.print("FEEDBACK:"); 
//     Serial.print(feedbackData.rpmFL);   Serial.print(",");
//     Serial.print(feedbackData.rpmFR);   Serial.print(",");
//     Serial.print(feedbackData.rpmRL);   Serial.print(",");
//     Serial.print(feedbackData.rpmRR);   Serial.print(",");
//     Serial.print(TL.rpm);               Serial.print(",");
//     Serial.print(TR.rpm);               Serial.print(","); 
//     Serial.print(feedbackData.countFL); Serial.print(",");
//     Serial.print(feedbackData.countFR); Serial.print(",");
//     Serial.print(feedbackData.countRL); Serial.print(",");
//     Serial.print(feedbackData.countRR); Serial.print(","); 
//     Serial.print(TL.encoderCount);      Serial.print(","); 
//     Serial.println(TR.encoderCount);      Serial.print(",");
//     // Serial.println(mpu.getYaw());       // Data ke-13: Yaw dari MPU
// }

// unsigned long lastTime = 0;
// float dt = 0.1;

// void setup() {
//     Serial.begin(115200);
    
//     // Setup I2C & MPU9250
//     // Wire.begin();
//     delay(2000);
//     // if (!mpu.setup(0x68)) {
//     //     while (1) { Serial.println("MPU Fail"); delay(5000); }
//     // }
//     // Serial.println("Calibrating... Do not move");
//     // mpu.calibrateAccelGyro();
    
//     // Setup Pin Lokal
//     Motor *localMotors[] = {&TR, &TL};
//     for(int i=0; i<2; i++) {
//         pinMode(localMotors[i]->encA, INPUT); pinMode(localMotors[i]->encB, INPUT);
//         pinMode(localMotors[i]->pwmPin, OUTPUT); pinMode(localMotors[i]->in1, OUTPUT); pinMode(localMotors[i]->in2, OUTPUT);
//     }
//     attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//     WiFi.mode(WIFI_STA);
//     if (esp_now_init() != ESP_OK) {
//         Serial.println("Error initializing ESP-NOW");
//         return;
//     }
    
//     // 1. Register peer untuk ESP Slave Pertama
//     esp_now_peer_info_t peerInfoSlave = {};
//     memcpy(peerInfoSlave.peer_addr, slaveAddress, 6);
//     peerInfoSlave.channel = 0; 
//     peerInfoSlave.encrypt = false;
//     if (esp_now_add_peer(&peerInfoSlave) != ESP_OK) {
//         Serial.println("Gagal menambahkan ESP Slave");
//     }

//     // 2. Register peer untuk ESP ke-3
//     esp_now_peer_info_t peerInfoEsp3 = {};
//     memcpy(peerInfoEsp3.peer_addr, esp3Address, 6);
//     peerInfoEsp3.channel = 0; 
//     peerInfoEsp3.encrypt = false;
//     if (esp_now_add_peer(&peerInfoEsp3) != ESP_OK) {
//         Serial.println("Gagal menambahkan ESP ke-3");
//     }

//     // Register callback penerima
//     esp_now_register_recv_cb(OnDataRecv);
// }

// void loop() {
//     // mpu.update(); // Update data sensor setiap saat

//     // Terima dari ROS
//     if (Serial.available() > 0) {
//         String data = Serial.readStringUntil('\n');
//         float v[9]; // Array untuk 10 data (indeks 0 - 9)
//         int idx = 0; int pos = 0;
        
//         // Parsing data sampai koma ke-9
//         while ((pos = data.indexOf(',')) != -1 && idx < 9) { // <-- UBAH idx < 8 MENJADI idx < 9
//             v[idx++] = data.substring(0, pos).toFloat();
//             data = data.substring(pos + 1);
//         }
//         v[idx] = data.toFloat(); // Memasukkan data terakhir ke v[9]

//         // 1. Assign ke Motor & Struct Slave Pertama
//         motorToSlave.fl = (int)v[0]; 
//         motorToSlave.fr = (int)v[1];
//         motorToSlave.rl = (int)v[2]; 
//         motorToSlave.rr = (int)v[3];
//         TL.setpoint = v[4]; 
//         TR.setpoint = v[5];
        
//         // Kirim ke ESP Slave Pertama
//         esp_now_send(slaveAddress, (uint8_t *) &motorToSlave, sizeof(motorToSlave));

//         // 2. Assign data ke-7 sampai ke-10 ke Struct ESP ke-3
//         dataKeEsp3.data1 = v[5];
//         dataKeEsp3.data2 = v[7];
//         dataKeEsp3.data3 = v[8];
//         dataKeEsp3.data4 = v[9]; // Sekarang v[9] sudah terbaca dengan benar

//         // Kirim ke ESP ke-3
//         esp_now_send(esp3Address, (uint8_t *) &dataKeEsp3, sizeof(dataKeEsp3));
//     }

//     // PID Loop tiap 100ms
//     unsigned long now = millis();
//     if (now - lastTime >= 100) {
//         updateMotorPID(TR, dt);
//         updateMotorPID(TL, dt);
//         lastTime = now;
//     }
// }