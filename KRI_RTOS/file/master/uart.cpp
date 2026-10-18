// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// // #include <Wire.h>          
// // #include "MPU9250.h" // Library Hideaki Tai

// // Inisialisasi Sensor
// // MPU9250 mpu;

// // ======================================================
// // DEFINISI PIN & UART
// // ======================================================
// #define RX2_PIN 16
// #define TX2_PIN 17

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
// // STRUKTUR DATA KOMUNIKASI
// // ======================================================
// // Struktur data kirim ke Slave
// typedef struct struct_message {
//     int fl; int fr; int rl; int rr;
// } struct_message;

// // Struktur feedback dari Slave
// typedef struct struct_feedback {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback;

// struct_message motorToSlave;

// // ======================================================
// // STRUKTUR MOTOR LOKAL & OBJEK RTOS
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

// SemaphoreHandle_t setpointMutex;  
// QueueHandle_t feedbackQueue;      

// // ======================================================
// // FUNGSI KONTROL MOTOR
// // ======================================================
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

// // ======================================================
// // TASK RTOS 1: Baca Data dari ROS (Serial 0) & Kirim ke Slave (UART 2)
// // ======================================================
// void TaskSerialRead(void *pvParameters) {
//     for (;;) {
//         if (Serial.available() > 0) {
//             String data = Serial.readStringUntil('\n');
//             float v[6] = {0}; // Inisialisasi awal dengan 0
//             int idx = 0; int pos = 0;
            
//             // Membaca 6 data setpoint dari ROS
//             while ((pos = data.indexOf(',')) != -1 && idx < 5) {
//                 v[idx++] = data.substring(0, pos).toFloat();
//                 data = data.substring(pos + 1);
//             }
//             if (idx == 5) {
//                 v[idx] = data.toFloat();
//             }

//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 // Sekarang indeks v[0] sampai v[5] sudah sinkron dengan Python ROS
//                 motorToSlave.fl = (int)v[0]; 
//                 motorToSlave.fr = (int)v[1];
//                 motorToSlave.rl = (int)v[2]; 
//                 motorToSlave.rr = (int)v[3];
                
//                 TL.setpoint = v[4]; // Motor lokal kiri depan/atas
//                 TR.setpoint = v[5]; // Motor lokal kanan depan/atas
//                 xSemaphoreGive(setpointMutex);
                
//                 // Kirim biner data struct ke Slave via UART 2
//                 Serial2.write((uint8_t *)&motorToSlave, sizeof(motorToSlave));
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK RTOS 2: Terima Data dari Slave (UART 2) & Masukkan ke Queue
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//     struct_feedback tempData;
//     for (;;) {
//         // Cek apakah ada data biner masuk yang ukurannya pas dengan struct feedback
//         if (Serial2.available() >= sizeof(struct_feedback)) {
//             Serial2.readBytes((uint8_t*)&tempData, sizeof(struct_feedback));
            
//             // Masukkan data ke Antrean (Queue) untuk diprint oleh TaskSerialWrite
//             // Timeout di-set ke portMAX_DELAY jika antrean sedang penuh
//             xQueueSend(feedbackQueue, &tempData, portMAX_DELAY);
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK RTOS 3: Baca Queue & Tulis Feedback ke ROS (Serial 0)
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     struct_feedback receivedFeedback;
//     for (;;) {
//         // Task akan menunggu santai sampai ada data masuk di Queue
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
//             Serial.println(TR.encoderCount);          Serial.println(); 
//             // Serial.println(mpu.getYaw()); 
//         }
//     }
// }

// // ======================================================
// // TASK RTOS 4: PID Kontrol Motor Master
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
//     // Komunikasi dengan PC/ROS (Kabel USB)
//     Serial.begin(115200);
    
//     // Komunikasi dengan ESP32 Slave (Kabel Jumper)
//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
    
//     // Inisialisasi Mutex & Queue
//     setpointMutex = xSemaphoreCreateMutex();
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

//     // Pembuatan Task RTOS
//     // Komunikasi di Core 0
//     xTaskCreatePinnedToCore(TaskSerialRead,  "SerialRead",  4096, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskUARTReceive, "UARTReceive", 4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskSerialWrite, "SerialWrite", 4096, NULL, 1, NULL, 0); 
    
//     // Motor Kontrol di Core 1 (Agar PID tidak tersendat proses komunikasi)
//     xTaskCreatePinnedToCore(TaskMotorPID,    "MotorPID",    4096, NULL, 2, NULL, 1); 
// }

// void loop() {
//     vTaskDelete(NULL); // Hapus default loop task
// }