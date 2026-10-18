// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <Arduino.h>

// // ======================================================
// // DEFINISI PIN & HARDWARE
// // ======================================================
// // Definisi Pin UART 2
// #define RX2_PIN 16
// #define TX2_PIN 17

// // md2 (reverse right)
// #define RL_PWM 18
// #define RL_IN1 2
// #define RL_IN2 4 
// #define RL_ENC_A 33
// #define RL_ENC_B 32

// // md1 (Front Left)
// #define RR_PWM 19
// #define RR_IN1 5
// #define RR_IN2 23
// #define RR_ENC_A 35
// #define RR_ENC_B 34

// // md3 (Front Right)
// #define FR_PWM 22
// #define FR_IN1 14
// #define FR_IN2 13
// #define FR_ENC_A 39
// #define FR_ENC_B 36

// // abaikan aja
// #define FL_PWM 21
// #define FL_IN1 27
// #define FL_IN2 26
// #define FL_ENC_A 15
// #define FL_ENC_B 25

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

// typedef struct __attribute__((packed)) struct_feedback {
//     long countFR; long countRL; long countRR;
// } struct_feedback;

// struct_feedback feedbackToMaster;

// // ======================================================
// // FILTER & WATCHDOG VARIABLES
// // ======================================================
// volatile unsigned long lastDataReceivedTime = 0;
// const int MAX_SAFE_RPM = 500; // Sesuaikan dengan RPM/setpoint maksimal motor Anda

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

// // Inisialisasi Objek Motor dengan Setpoint awal = 0
// Motor RL = { RL_PWM, RL_IN1, RL_IN2, RL_ENC_A, RL_ENC_B, 0, 0, 0, 385,  0, 0, 0.2, 2.0, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor RR = { RR_PWM, RR_IN1, RR_IN2, RR_ENC_A, RR_ENC_B, 0, 0, 0, 385,  0, 0, 0.2, 2.0, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor FR = { FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 385, 0, 0, 0.2, 2.0, 0.03, 0, 0, 0, 0, 0, 0 };
// Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 385,  0, 0, 0.2, 2.0, 0.03, 0, 0, 0, 0, 0, 0 };

// // Forward Declaration
// void updateEncoder(Motor &m);

// // ISR Handlers
// void IRAM_ATTR encoderRL() { updateEncoder(RL); }
// void IRAM_ATTR encoderRR() { updateEncoder(RR); }
// void IRAM_ATTR encoderFR() { updateEncoder(FR); }
// void IRAM_ATTR encoderFL() { updateEncoder(FL); }

// // ======================================================
// // FUNGSI KONTROL & SENSOR
// // ======================================================
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
//     m.integral = 0; // Reset integral agar tidak menumpuk saat diam
//   }
//   m.lastError = m.error;
// }

// // ======================================================
// // TASK RTOS 1: Menerima Data UART (Dengan Filter)
// // ======================================================
// void TaskUARTReceive(void *pvParameters) {
//   for (;;) {
//     if (Serial2.available() >= sizeof(struct_message)) {
//       struct_message tempMsg;
      
//       Serial2.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

//       // FILTER 1: Sanity Check (Pastikan nilai setpoint masuk akal)
//       if (abs(tempMsg.fl) <= MAX_SAFE_RPM && 
//           abs(tempMsg.fr) <= MAX_SAFE_RPM && 
//           abs(tempMsg.rl) <= MAX_SAFE_RPM && 
//           abs(tempMsg.rr) <= MAX_SAFE_RPM) {
            
//         if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
//           FL.setpoint = tempMsg.fl;
//           FR.setpoint = tempMsg.fr;
//           RL.setpoint = tempMsg.rl;
//           RR.setpoint = tempMsg.rr;
          
//           lastDataReceivedTime = millis(); // Update timer karena mendapat data valid
//           xSemaphoreGive(setpointMutex);
//         }
//       } else {
//         // Data terdeteksi sebagai sampah (noise/bootloader master), flush sisa buffer serial
//         while(Serial2.available() > 0) {
//           Serial2.read();
//         }
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(10)); // Mencegah Task Monopoli CPU
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
      
//       // FILTER 2: Watchdog Timeout (Master restart/kabel putus)
//       // Jika tidak ada data valid selama 500ms, paksa setpoint menjadi 0
//       if (millis() - lastDataReceivedTime > 500) {
//         FL.setpoint = 0;
//         FR.setpoint = 0;
//         RL.setpoint = 0;
//         RR.setpoint = 0;
//       }

//       updateMotorPID(FL, dt);
//       updateMotorPID(FR, dt);
//       updateMotorPID(RL, dt);
//       updateMotorPID(RR, dt);
//       xSemaphoreGive(setpointMutex);
//     }

//     // Update data feedback untuk master
//     feedbackToMaster.countFR = FR.encoderCount;
//     feedbackToMaster.countRR = RR.encoderCount;
//     feedbackToMaster.countRL = RL.encoderCount; 

//     Serial2.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));

//     // Print ke Serial Monitor untuk debugging lokal
//     // Serial.printf("Setpoint -> FR: %.2f | FL: %.2f | RR: %.2f | RL: %.2f\n", FR.setpoint, FL.setpoint, RR.setpoint, RL.setpoint);
//     Serial.print("RL: "); Serial.print(RL.rpm); 
//     Serial.print(" | RR: "); Serial.print(RR.rpm); 
//     Serial.print(" | FR: "); Serial.println(FR.rpm); 

//     vTaskDelayUntil(&xLastWakeTime, xFrequency);
//   }
// }

// // ======================================================
// // SETUP & LOOP
// // ======================================================
// void setup() {
//   // Matikan driver motor sesegera mungkin saat boot untuk menghindari glitch
//   int motorPins[] = {RL_PWM, RL_IN1, RL_IN2, RR_PWM, RR_IN1, RR_IN2, 
//                      FR_PWM, FR_IN1, FR_IN2, FL_PWM, FL_IN1, FL_IN2};
//   for (int i = 0; i < 12; i++) {
//     pinMode(motorPins[i], OUTPUT);
//     digitalWrite(motorPins[i], LOW);
//   }

//   Serial.begin(115200);
//   Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

//   // Flush data sampah di buffer serial akibat proses reset/booting awal
//   while(Serial2.available() > 0) {
//     Serial2.read();
//   }

//   setpointMutex = xSemaphoreCreateMutex();
//   lastDataReceivedTime = millis(); // Inisialisasi awal watchdog

//   Motor *motors[] = { &RL, &RR, &FR, &FL };
//   for (int i = 0; i < 4; i++) {
//     pinMode(motors[i]->encA, INPUT_PULLUP);
//     pinMode(motors[i]->encB, INPUT_PULLUP);
//   }

//   attachInterrupt(digitalPinToInterrupt(RL_ENC_A), encoderRL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RL_ENC_B), encoderRL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RR_ENC_A), encoderRR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(RR_ENC_B), encoderRR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

//   xTaskCreatePinnedToCore(TaskUARTReceive, "TaskUART", 2048, NULL, 1, NULL, 0);
//   xTaskCreatePinnedToCore(TaskMotorControl, "TaskMotor", 4096, NULL, 2, NULL, 1);
// }

// void loop() {
//   // Dikosongkan karena menggunakan RTOS Tasks
//   vTaskDelay(pdMS_TO_TICKS(1000)); 
// }