// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>

// // PIN UART untuk Slave 2
// #define RX1_PIN 25
// #define TX1_PIN 26

// // Definisi Pin Motor FL (Silakan sesuaikan dengan pin physical Anda)
// #define FL_PWM     22
// #define FL_IN1     13
// #define FL_IN2     14
// #define FL_ENC_A   39
// #define FL_ENC_B   36

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI (Wajib Sinkron dengan Master)
// // ======================================================
// typedef struct struct_message {
//   float data1; // Digunakan sebagai SETPOINT Motor FL
//   float data2;
//   float data3;
//   float data4;
// } struct_message;

// struct_message incomingData;

// typedef struct struct_feedback {
//   float out1;  // Digunakan untuk mengirim balik RPM Motor FL
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
//   int pwmPin;
//   int in1;
//   int in2;
//   int encA;
//   int encB;
//   volatile long encoderCount;
//   volatile int lastEncoded;
//   long lastCount;
//   float ppr;
//   float setpoint;
//   float rpm;
//   float Kp;
//   float Ki;
//   float Kd;
//   float error;
//   float lastError;
//   float integral;
//   float derivative;
//   float control;
//   int pwmOutput;
// };

// // Inisialisasi Objek Motor FL (Gunakan parameter PID & PPR andalan Anda)
// Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1000, 0, 0, 0.1, 6.5, 0.03, 0, 0, 0, 0, 0, 0 };

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
//   else {
//     putarMotor(m, 0);
//     m.integral = 0;
//   }
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
//         FL.setpoint = incomingData.data1; // Setpoint FL mengambil nilai dari data1
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
//   float dt = 0.1; // Sesuai interval 100ms (0.1 detik)
//   const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   for (;;) {
//     if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
//       updateMotorPID(FL, dt); // Jalankan kalkulasi PID untuk FL
      
//       // Kemas feedback untuk dikirim balik ke master
//       feedbackToMaster.out1 = FL.rpm; 
//       feedbackToMaster.out2 = 0.0;
//       feedbackToMaster.out3 = 0.0; 
//       feedbackToMaster.out4 = 0.0;
      
//       xSemaphoreGive(dataMutex);
//     }

//     // Kirim struct biner feedback ke Master via UART
//     Serial1.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));

//     // Debugging via Serial Monitor USB
//     Serial.printf("Setpoint FL (data1): %.2f | Real RPM FL: %.2f | Enc: %ld\n", FL.setpoint, FL.rpm, FL.encoderCount);

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// void setup() {
//   Serial.begin(115200);
  
//   // Menggunakan Serial1 untuk komunikasi antar-mikrokontroler
//   Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

//   dataMutex = xSemaphoreCreateMutex();

//   // Konfigurasi IO Hardware Motor FL
//   pinMode(FL.encA, INPUT);
//   pinMode(FL.encB, INPUT);
//   pinMode(FL.pwmPin, OUTPUT);
//   pinMode(FL.in1, OUTPUT);
//   pinMode(FL.in2, OUTPUT);

//   // Pasang Interrupt Encoder
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//   // Task UART Receive di Core 0 (Prioritas 1)
//   xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRecv", 2048, NULL, 1, NULL, 0);

//   // Task Motor Control & Feedback di Core 1 (Prioritas 2 untuk menjamin kestabilan timing PID)
//   xTaskCreatePinnedToCore(TaskMotorAndFeedback, "MotorPID", 4096, NULL, 2, NULL, 1);
// }

// void loop() {
//   vTaskDelete(NULL); 
// }