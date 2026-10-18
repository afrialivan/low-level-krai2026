// #include <esp_now.h>
// #include <WiFi.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

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

// // MAC Address Slave
// uint8_t slaveAddress[] = {0x20, 0xE7, 0xC8, 0xA7, 0x9B, 0x0C}; 

// // Struktur data kirim ke Slave
// typedef struct struct_message {
//     int fl; int fr; int rl; int rr;
// } struct_message;

// // Struktur feedback dari Slave
// typedef struct struct_feedback {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback;

// // Struktur untuk Motor Lokal
// struct Motor {
//     int pwmPin; int in1; int in2; int encA; int encB;
//     volatile long encoderCount;
//     volatile int lastEncoded;
//     long lastCount;
//     float ppr; float setpoint; float rpm;
//     float Kp, Ki, Kd, error, lastError, integral, derivative, control;
//     int pwmOutput;
// };

// struct_message motorToSlave;

// // Inisialisasi Motor Lokal (Roda Depan/Top)
// Motor TR = {FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 1001, 0, 0, 1.0, 3.5, 0.03, 0, 0, 0, 0, 0, 0};
// Motor TL = {FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1001, 0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0};

// // ======================================================
// // OBJEK RTOS (MUTEX & QUEUE)
// // ======================================================
// SemaphoreHandle_t setpointMutex;  // Mengamankan data setpoint dari ROS -> Task Motor
// QueueHandle_t feedbackQueue;      // Antrean data dari ESP-NOW Callback -> Task Serial

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

// // Callback ESP-NOW: Memasukkan data ke Queue (BUKAN print serial)
// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//     struct_feedback tempData;
//     memcpy(&tempData, incomingData, sizeof(tempData));
    
//     // Kirim data ke Queue agar diproses oleh TaskSerialWrite secara aman
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     xQueueSendFromISR(feedbackQueue, &tempData, &xHigherPriorityTaskWoken);
    
//     if (xHigherPriorityTaskWoken) {
//         portYIELD_FROM_ISR();
//     }
// }

// // ======================================================
// // TASK RTOS 1: Baca Data dari ROS (Serial RX)
// // ======================================================
// void TaskSerialRead(void *pvParameters) {
//     for (;;) {
//         if (Serial.available() > 0) {
//             String data = Serial.readStringUntil('\n');
//             float v[6]; int idx = 0; int pos = 0;
            
//             while ((pos = data.indexOf(',')) != -1 && idx < 5) {
//                 v[idx++] = data.substring(0, pos).toFloat();
//                 data = data.substring(pos + 1);
//             }
//             v[idx] = data.toFloat();

//             // Ambil Mutex sebelum mengubah data kecepatan/setpoint
//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 motorToSlave.fl = (int)v[0]; 
//                 motorToSlave.fr = (int)v[1];
//                 motorToSlave.rl = (int)v[2]; 
//                 motorToSlave.rr = (int)v[3];
//                 TL.setpoint = v[4]; 
//                 TR.setpoint = v[5];
//                 xSemaphoreGive(setpointMutex);
                
//                 // Kirim data ke slave (ESP-NOW aman dipanggil dari Task)
//                 esp_now_send(slaveAddress, (uint8_t *) &motorToSlave, sizeof(motorToSlave));
//             }
//         }
//         // Istirahat 10ms agar tidak trigger Watchdog
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK RTOS 2: Tulis Data ke ROS (Serial TX)
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     struct_feedback receivedFeedback;
//     for (;;) {
//         // Task akan tertidur sampai ada data di dalam Queue dari callback ESP-NOW
//         if (xQueueReceive(feedbackQueue, &receivedFeedback, portMAX_DELAY) == pdPASS) {
            
//             Serial.print("FEEDBACK:"); 
//             Serial.print(receivedFeedback.rpmFL);   Serial.print(",");
//             Serial.print(receivedFeedback.rpmFR);   Serial.print(",");
//             Serial.print(receivedFeedback.rpmRL);   Serial.print(",");
//             Serial.print(receivedFeedback.rpmRR);   Serial.print(",");
//             Serial.print(TL.rpm);                   Serial.print(",");
//             Serial.print(TR.rpm);                   Serial.print(","); 
//             Serial.print(receivedFeedback.countFL); Serial.print(",");
//             Serial.print(receivedFeedback.countFR); Serial.print(",");
//             Serial.print(receivedFeedback.countRL); Serial.print(",");
//             Serial.print(receivedFeedback.countRR); Serial.print(","); 
//             Serial.print(TL.encoderCount);          Serial.print(","); 
//             Serial.print(TR.encoderCount);          Serial.println(); // Gunakan println di akhir
//             // Serial.println(mpu.getYaw()); 
//         }
//     }
// }

// // ======================================================
// // TASK RTOS 3: PID Kontrol Motor Master
// // ======================================================
// void TaskMotorPID(void *pvParameters) {
//     float dt = 0.1;
//     const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//     TickType_t xLastWakeTime = xTaskGetTickCount();

//     for (;;) {
//         // Ambil Mutex untuk membaca setpoint dengan aman
//         if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//             updateMotorPID(TR, dt);
//             updateMotorPID(TL, dt);
//             xSemaphoreGive(setpointMutex);
//         }

//         // mpu.update(); // Update MPU jika ingin digabungkan ke task ini atau task terpisah

//         // vTaskDelayUntil memastikan loop selalu berjalan presisi 100ms
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// void setup() {
//     Serial.begin(115200);
    
//     // Inisialisasi Mutex & Queue
//     setpointMutex = xSemaphoreCreateMutex();
//     // Queue sepanjang 5 item untuk menampung data feedback
//     feedbackQueue = xQueueCreate(5, sizeof(struct_feedback)); 

//     // Setup I2C & MPU9250
//     // Wire.begin();
//     delay(2000);
    
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
//     if (esp_now_init() != ESP_OK) return;
    
//     esp_now_peer_info_t peerInfo = {};
//     memcpy(peerInfo.peer_addr, slaveAddress, 6);
//     peerInfo.channel = 0; peerInfo.encrypt = false;
//     esp_now_add_peer(&peerInfo);
//     esp_now_register_recv_cb(OnDataRecv);

//     // Pembuatan Task RTOS
//     xTaskCreatePinnedToCore(TaskSerialRead,  "SerialRead",  4096, NULL, 1, NULL, 0); // Di Core 0 (Komunikasi)
//     xTaskCreatePinnedToCore(TaskSerialWrite, "SerialWrite", 4096, NULL, 1, NULL, 0); // Di Core 0 (Komunikasi)
//     xTaskCreatePinnedToCore(TaskMotorPID,    "MotorPID",    4096, NULL, 2, NULL, 1); // Di Core 1 (Prioritas tinggi)
// }

// void loop() {
//     vTaskDelete(NULL); // Hapus default loop task
// }