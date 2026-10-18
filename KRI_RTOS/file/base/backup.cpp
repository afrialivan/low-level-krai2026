// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>

// // Definisi Pin UART 2
// #define RX2_PIN 16
// #define TX2_PIN 17

// //md3
// #define RL_PWM 22
// #define RL_IN1 14
// #define RL_IN2 13
// #define RL_ENC_A 34
// #define RL_ENC_B 35

// // md4
// #define RR_PWM 7
// #define RR_IN1 27
// #define RR_IN2 26
// #define RR_ENC_A 39
// #define RR_ENC_B 36

// // md2
// #define FR_PWM 18
// #define FR_IN1 2
// #define FR_IN2 4
// #define FR_ENC_A 25
// #define FR_ENC_B 15

// // md1
// #define FL_PWM 19
// #define FL_IN1 5
// #define FL_IN2 23
// #define FL_ENC_A 32
// #define FL_ENC_B 33

// // ======================================================
// // STRUKTUR DATA KOMUNIKASI
// // ======================================================
// typedef struct struct_message {
//   int fl;
//   int fr;
//   int rl;
//   int rr;
// } struct_message;

// struct_message incomingData;

// typedef struct struct_feedback {
//     float rpmFL; float rpmFR; float rpmRL; float rpmRR;
//     long countFL; long countFR; long countRL; long countRR;
// } struct_feedback;

// struct_feedback feedbackToMaster;

// // ======================================================
// // STRUKTUR MOTOR & MUTEX RTOS
// // ======================================================
// SemaphoreHandle_t setpointMutex;

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

// // Inisialisasi Objek Motor
// Motor RL = { RL_PWM, RL_IN1, RL_IN2, RL_ENC_A, RL_ENC_B, 0, 0, 0, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor RR = { RR_PWM, RR_IN1, RR_IN2, RR_ENC_A, RR_ENC_B, 0, 0, 0, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor FR = { FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 40600, 0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 1420,  0, 0, 2.1, 2.5, 0.03, 0, 0, 0, 0, 0, 0 };

// // ISR Handlers
// void IRAM_ATTR encoderRL() { updateEncoder(RL); }
// void IRAM_ATTR encoderRR() { updateEncoder(RR); }
// void IRAM_ATTR encoderFR() { updateEncoder(FR); }
// void IRAM_ATTR encoderFL() { updateEncoder(FL); }

// void updateEncoder(Motor &m) {
//   int A = digitalRead(m.encA);
//   int B = digitalRead(m.encB);
//   int encoded = (A << 1) | B;
//   int sum = (m.lastEncoded << 2) | encoded;
//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) m.encoderCount++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) m.encoderCount--;
//   m.lastEncoded = encoded;
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
// // TASK RTOS 1: Menerima Data UART (Pengganti ESP-NOW Callback)
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//   for (;;) {
//     // Mengecek apakah ada data biner masuk yang cukup untuk mengisi struct_message
//     if (Serial2.available() >= sizeof(struct_message)) {
//       struct_message tempMsg;
      
//       // Membaca raw byte langsung ke dalam struct
//       Serial2.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

//       // Ambil Mutex sebelum memperbarui nilai Setpoint
//       if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//         FL.setpoint = tempMsg.fl;
//         FR.setpoint = tempMsg.fr;
//         RL.setpoint = tempMsg.rl;
//         RR.setpoint = tempMsg.rr;
        
//         xSemaphoreGive(setpointMutex);
//       }
//     }
//     // Istirahatkan Task selama 10ms agar tidak memonopoli CPU (Watchdog)
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// }

// // ======================================================
// // TASK RTOS 2: Kontrol Motor & Mengirim Feedback via UART
// // ======================================================
// void TaskMotorControl(void *pvParameters) {
//   float dt = 0.1;
//   const TickType_t xFrequency = pdMS_TO_TICKS(100); 
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   for (;;) {
//     if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//       updateMotorPID(FL, dt);
//       updateMotorPID(FR, dt);
//       updateMotorPID(RL, dt);
//       updateMotorPID(RR, dt);
//       xSemaphoreGive(setpointMutex);
//     }

//     // Siapkan Feedback
//     feedbackToMaster.rpmFL = FL.rpm; feedbackToMaster.rpmFR = FR.rpm;
//     feedbackToMaster.rpmRL = RL.rpm; feedbackToMaster.rpmRR = RR.rpm;
//     feedbackToMaster.countFL = FL.encoderCount; feedbackToMaster.countFR = FR.encoderCount;
//     feedbackToMaster.countRL = RL.encoderCount; feedbackToMaster.countRR = RR.encoderCount;

//     // Kirim Feedback ke Master via UART2 (mengirim data biner)
//     Serial2.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));

//     // Debugging via USB Serial (Opsional)
//     Serial.printf("Setpoint -> FR: %.2f | FL: %.2f | RR: %.2f | RL: %.2f\n", FR.setpoint, FL.setpoint, RR.setpoint, RL.setpoint);

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// void setup() {
//   // Serial bawaan (UART 0) untuk debugging ke komputer
//   Serial.begin(115200);

//   // Inisialisasi UART 2 untuk komunikasi antar-mikrokontroler
//   // Baud rate: 115200, Format: 8 data bits, No parity, 1 stop bit
//   Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

//   setpointMutex = xSemaphoreCreateMutex();

//   Motor *motors[] = { &RL, &RR, &FR, &FL };
//   for (int i = 0; i < 4; i++) {
//     pinMode(motors[i]->encA, INPUT);
//     pinMode(motors[i]->encB, INPUT);
//     pinMode(motors[i]->pwmPin, OUTPUT);
//     pinMode(motors[i]->in1, OUTPUT);
//     pinMode(motors[i]->in2, OUTPUT);
//   }

//   attachInterrupt(digitalPinToInterrupt(RL_ENC_A), encoderRL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RL_ENC_B), encoderRL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RR_ENC_A), encoderRR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RR_ENC_B), encoderRR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//   // Task UART (Menerima Data), Pin ke Core 0
//   xTaskCreatePinnedToCore(
//     TaskUARTReceive, 
//     "TaskUART",      
//     2048,            
//     NULL,            
//     1,               
//     NULL,            
//     0                
//   );

//   // Task Motor Control (Kirim Data & PID), Pin ke Core 1
//   xTaskCreatePinnedToCore(
//     TaskMotorControl, 
//     "TaskMotor",      
//     4096,             
//     NULL,             
//     2,                // Prioritas sedikit lebih tinggi agar presisi waktu dt (100ms) terjamin
//     NULL,             
//     1                 
//   );
// }

// // void loop() {
// //   // Matikan task default untuk menghemat RAM
// //   vTaskDelete(NULL); 
// // }

// void loop() {
//   vTaskDelay(pdMS_TO_TICKS(1000)); // Biarkan loop tidur selamanya
// }