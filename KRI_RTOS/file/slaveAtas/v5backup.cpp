// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <ESP32Servo.h>  // Tambahan library servo

// // PIN UART untuk Slave 2
// #define RX1_PIN 25
// #define TX1_PIN 26

// // Definisi Pin Motor FL
// #define FL_PWM     22
// #define FL_IN1     13
// #define FL_IN2     14
// #define FL_ENC_A   39
// #define FL_ENC_B   36

// // === TAMBAHAN: Definisi Pin Servo ===
// #define SERVO1_PIN  16
// #define SERVO2_PIN  21

// // ==== ==================================================
// // STRUKTUR DATA KOMUNIKASI (Wajib Sinkron dengan Master)
// // ======================================================
// typedef struct struct_message {
//   float data1;
//   float data2; // Digunakan sebagai trigger servo (1 = gerak ke 90°)
//   float data3;
//   float data4;
// } struct_message;

// struct_message incomingData;

// typedef struct struct_feedback {
//   float out1;
//   float out2;
//   float out3;
//   float out4;
// } struct_feedback;

// struct_feedback feedbackToMaster;

// // ======================================================
// // STRUKTUR MOTOR & MUTEX RTOS
// // ======================================================
// SemaphoreHandle_t dataMutex;

// struct Motor {
//   int pwmPin, in1, in2, encA, encB;
//   volatile long encoderCount;
//   volatile int lastEncoded;
//   long lastCount;
//   float ppr, setpoint, rpm;
//   float Kp, Ki, Kd;
//   float error, lastError, integral, derivative, control;
//   int pwmOutput;
// };

// Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1000, 0, 0, 0.1, 6.5, 0.03, 0, 0, 0, 0, 0, 0 };

// // === TAMBAHAN: Objek Servo & Flag Eksekusi Sekali ===
// Servo servo1;
// Servo servo2;
// bool servoTriggered = false; // Flag agar hanya eksekusi SEKALI

// // ======================================================
// // ISR & FUNGSI KONTROL MOTOR
// // ======================================================
// void IRAM_ATTR encoderFL() {
//   int A = digitalRead(FL.encA);
//   int B = digitalRead(FL.encB);
//   int encoded = (A << 1) | B;
//   int sum = (FL.lastEncoded << 2) | encoded;
//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) FL.encoderCount++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) FL.encoderCount--;
//   FL.lastEncoded = encoded;
// }

// void putarMotor(Motor &m, int kecepatan) {
//   int nilai_pwm = constrain(abs(kecepatan), 0, 255);
//   if (kecepatan > 0) {
//     digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, nilai_pwm);
//   } else if (kecepatan < 0) {
//     digitalWrite(m.in1, LOW); digitalWrite(m.in2, HIGH); analogWrite(m.pwmPin, nilai_pwm);
//   } else {
//     digitalWrite(m.in1, LOW); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, 0);
//   }
// }

// void updateMotorPID(Motor &m, float dt) {
//   long delta = m.encoderCount - m.lastCount;
//   m.lastCount = m.encoderCount;
//   m.rpm = (delta * 600.0) / m.ppr;
//   m.error = abs(m.setpoint) - abs(m.rpm);
//   m.integral = constrain(m.integral + (m.error * dt), -500, 500);
//   m.derivative = (m.error - m.lastError) / dt;
//   m.control = (m.Kp * m.error) + (m.Ki * m.integral) + (m.Kd * m.derivative);
//   m.pwmOutput = constrain(abs(m.control), 0, 255);
//   if (m.setpoint > 0) putarMotor(m, m.pwmOutput);
//   else if (m.setpoint < 0) putarMotor(m, -m.pwmOutput);
//   else { putarMotor(m, 0); m.integral = 0; }
//   m.lastError = m.error;
// }

// // ======================================================
// // TASK RTOS 1: Menerima Data Setpoint dari Master via UART
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//   for (;;) {
//     if (Serial1.available() >= sizeof(struct_message)) {
//       struct_message tempMsg;
//       Serial1.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

//       if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//         incomingData = tempMsg;
//         FL.setpoint = incomingData.data1;
//         xSemaphoreGive(dataMutex);
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// }

// // ======================================================
// // TASK RTOS 2: Eksekusi PID Motor & Kirim Feedback
// // ======================================================
// void TaskMotorAndFeedback(void *pvParameters) {
//   float dt = 0.1;
//   const TickType_t xFrequency = pdMS_TO_TICKS(100);
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   for (;;) {
//     if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//       updateMotorPID(FL, dt);
//       feedbackToMaster.out1 = FL.encoderCount;
//       feedbackToMaster.out2 = 0.0;
//       feedbackToMaster.out3 = 0.0;
//       feedbackToMaster.out4 = 0.0;
//       xSemaphoreGive(dataMutex);
//     }

//     Serial1.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));
//     Serial.printf("Setpoint FL: %.2f | RPM FL: %.2f | Enc: %ld, | proxy: %f,\n", FL.setpoint, FL.rpm, FL.encoderCount, incomingData.data2);

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// // ======================================================
// // TASK RTOS 3 (TAMBAHAN): Monitor data2 & Kontrol Servo
// // ======================================================
// void TaskServoControl(void *pvParameters) {
//   for (;;) {
//     if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//       float triggerValue = incomingData.data2;
//       xSemaphoreGive(dataMutex);

//       // Eksekusi HANYA jika data2 == 1 DAN belum pernah dijalankan
//       if (triggerValue == 1.0f && !servoTriggered) {
//         // servo1.write(180);
//         // servo2.write(180);
//         vTaskDelay(pdMS_TO_TICKS(3000));
//         servo2.write(180);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//         servo1.write(80);
//         servoTriggered = true; // Kunci, tidak akan eksekusi lagi
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(50)); // Cek setiap 50ms, cukup responsif
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//   dataMutex = xSemaphoreCreateMutex();

//   // Konfigurasi IO Motor FL
//   pinMode(FL.encA, INPUT);
//   pinMode(FL.encB, INPUT);
//   pinMode(FL.pwmPin, OUTPUT);
//   pinMode(FL.in1, OUTPUT);
//   pinMode(FL.in2, OUTPUT);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//   // === TAMBAHAN: Inisialisasi Servo ke posisi DEFAULT 180° ===
//   servo1.attach(SERVO1_PIN);
//   servo2.attach(SERVO2_PIN);
//   servo1.write(180);
//   servo2.write(30);
//   // Task UART Receive → Core 0, Prioritas 1
//   xTaskCreatePinnedToCore(TaskUARTReceive,    "UARTRecv",  2048, NULL, 1, NULL, 0);

//   // Task Motor PID & Feedback → Core 1, Prioritas 2
//   xTaskCreatePinnedToCore(TaskMotorAndFeedback, "MotorPID", 4096, NULL, 2, NULL, 1);

//   // Task Servo Control → Core 0, Prioritas 1
//   xTaskCreatePinnedToCore(TaskServoControl,   "ServoCtrl", 2048, NULL, 1, NULL, 0);
// }

// void loop() {
//   vTaskDelete(NULL);
// }