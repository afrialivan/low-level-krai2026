// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/queue.h>

// // Port UART 2 -> Terhubung ke Slave 1
// #define RX2_PIN 16
// #define TX2_PIN 17

// // Port UART 1 -> Terhubung ke Slave 2 (Gunakan pin kosong bebas konflik)
// #define RX1_PIN 25
// #define TX1_PIN 26

// // Pin Motor Lokal Master
// #define FR_PWM   18
// #define FR_IN1   4
// #define FR_IN2   2
// #define FR_ENC_A 34
// #define FR_ENC_B 35

// #define FL_PWM   19
// #define FL_IN1   5
// #define FL_IN2   23
// #define FL_ENC_A 32
// #define FL_ENC_B 33

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI SLAVE 1
// // ======================================================
// typedef struct struct_message_s1 {
//     int fl; int fr; int rl; int rr;
// } struct_message_s1;

// typedef struct struct_feedback_s1 {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback_s1;

// struct_message_s1 motorToSlave1;

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI SLAVE 2
// // ======================================================
// typedef struct struct_message_s2 {
//     float data1; float data2; float data3; float data4;
// } struct_message_s2;

// typedef struct struct_feedback_s2 {
//     float out1; float out2; float out3; float out4;
// } struct_feedback_s2;

// struct_message_s2 motorToSlave2;

// // Global buffer feedback penampung terakhir untuk diprint ke ROS
// struct_feedback_s1 fbS1_latest;
// struct_feedback_s2 fbS2_latest;

// // ======================================================
// // MOTOR LOKAL & PENDUKUNG RTOS
// // ======================================================
// struct Motor {
//     int pwmPin; int in1; int in2; int encA; int encB;
//     volatile long encoderCount; volatile int lastEncoded; long lastCount;
//     float ppr; float setpoint; float rpm;
//     float Kp, Ki, Kd, error, lastError, integral, derivative, control;
//     int pwmOutput;
// };

// Motor TR = {FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 1001, 0, 0, 1.0, 3.5, 0.03, 0, 0, 0, 0, 0, 0};
// Motor TL = {FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1001, 0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0};

// SemaphoreHandle_t setpointMutex;  
// SemaphoreHandle_t feedbackMutex;

// void IRAM_ATTR encoderFR() { updateEncoder(TR); }
// void IRAM_ATTR encoderFL() { updateEncoder(TL); }

// void updateEncoder(Motor &m) {
//     int A = digitalRead(m.encA); int B = digitalRead(m.encB);
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
// // TASK 1: Baca Serial ROS & Kirim ke Slave 1 & Slave 2
// // ======================================================
// void TaskSerialRead(void *pvParameters) {
//     for (;;) {
//         if (Serial.available() > 0) {
//             String data = Serial.readStringUntil('\n');
//             float v[10] = {0}; int idx = 0; int pos = 0;
            
//             while ((pos = data.indexOf(',')) != -1 && idx < 9) {
//                 v[idx++] = data.substring(0, pos).toFloat();
//                 data = data.substring(pos + 1);
//             }
//             if(idx == 9) v[idx] = data.toFloat();

//             if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//                 // Parsing Data sesuai urutan ROS baru
//                 motorToSlave1.fl = (int)v[0]; 
//                 motorToSlave1.fr = (int)v[1];
//                 motorToSlave1.rl = (int)v[2]; 
//                 motorToSlave1.rr = (int)v[3];
                
//                 TL.setpoint = v[4]; 
//                 TR.setpoint = v[5];

//                 motorToSlave2.data1 = v[6];
//                 motorToSlave2.data2 = v[7];
//                 motorToSlave2.data3 = v[8];
//                 motorToSlave2.data4 = v[9];
//                 xSemaphoreGive(setpointMutex);
                
//                 // Kirim langsung ke masing-masing Slave lewat Jalur UART biner
//                 Serial2.write((uint8_t *)&motorToSlave1, sizeof(motorToSlave1));
//                 Serial1.write((uint8_t *)&motorToSlave2, sizeof(motorToSlave2));
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK 2: Terima Jalur UART dari Slave 1 & Slave 2
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//     struct_feedback_s1 s1Data;
//     struct_feedback_s2 s2Data;
//     for (;;) {
//         // Cek data masuk dari Slave 1
//         if (Serial2.available() >= sizeof(struct_feedback_s1)) {
//             Serial2.readBytes((uint8_t*)&s1Data, sizeof(struct_feedback_s1));
//             if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//                 fbS1_latest = s1Data;
//                 xSemaphoreGive(feedbackMutex);
//             }
//         }
//         // Cek data masuk dari Slave 2
//         if (Serial1.available() >= sizeof(struct_feedback_s2)) {
//             Serial1.readBytes((uint8_t*)&s2Data, sizeof(struct_feedback_s2));
//             if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//                 fbS2_latest = s2Data;
//                 xSemaphoreGive(feedbackMutex);
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// // ======================================================
// // TASK 3: Print Gabungan Feedback ke ROS Berdasarkan Timer (100ms)
// // ======================================================
// void TaskSerialWrite(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         if (xSemaphoreTake(feedbackMutex, portMAX_DELAY) == pdTRUE) {
//             Serial.print("FEEDBACK:"); 
//             Serial.print(fbS1_latest.rpmFL);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmFR);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmRL);   Serial.print(",");
//             Serial.print(fbS1_latest.rpmRR);   Serial.print(",");
//             Serial.print(TL.rpm);               Serial.print(",");
//             Serial.print(TR.rpm);               Serial.print(","); 
//             Serial.print(fbS1_latest.countFL); Serial.print(",");
//             Serial.print(fbS1_latest.countFR); Serial.print(",");
//             Serial.print(fbS1_latest.countRL); Serial.print(",");
//             Serial.print(fbS1_latest.countRR); Serial.print(","); 
//             Serial.print(TL.encoderCount);      Serial.print(","); 
//             Serial.print(TR.encoderCount);      Serial.print(",");
//             // Tambahkan data dari Slave 2 di akhir string koma
//             Serial.print(fbS2_latest.out1);     Serial.print(",");
//             Serial.print(fbS2_latest.out2);     Serial.print(",");
//             Serial.print(fbS2_latest.out3);     Serial.print(",");
//             Serial.println(fbS2_latest.out4);
            
//             xSemaphoreGive(feedbackMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// void TaskMotorPID(void *pvParameters) {
//     float dt = 0.1;
//     const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;) {
//         if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//             updateMotorPID(TR, dt);
//             updateMotorPID(TL, dt);
//             xSemaphoreGive(setpointMutex);
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }

// void setup() {
//     Serial.begin(115200);                               // Ke ROS PC
//     Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN); // Ke Slave 1
//     Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN); // Ke Slave 2
    
//     setpointMutex = xSemaphoreCreateMutex();
//     feedbackMutex = xSemaphoreCreateMutex();

//     Motor *localMotors[] = {&TR, &TL};
//     for(int i=0; i<2; i++) {
//         pinMode(localMotors[i]->encA, INPUT); pinMode(localMotors[i]->encB, INPUT);
//         pinMode(localMotors[i]->pwmPin, OUTPUT); pinMode(localMotors[i]->in1, OUTPUT); pinMode(localMotors[i]->in2, OUTPUT);
//     }
//     attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//     attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//     xTaskCreatePinnedToCore(TaskSerialRead,  "SerRead",  4096, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRcv",  4096, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(TaskSerialWrite, "SerWrite", 4096, NULL, 1, NULL, 0); 
//     xTaskCreatePinnedToCore(TaskMotorPID,    "MotPID",   4096, NULL, 2, NULL, 1); 
// }

// void loop() {
//     vTaskDelete(NULL);
// }