#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Arduino.h>

// Definisi Pin UART 2
#define RX2_PIN 16
#define TX2_PIN 17

// M1
#define RR_PWM 19
#define RR_IN1 5
#define RR_IN2 23
#define RR_ENC_A 34
#define RR_ENC_B 35

// M2
#define RL_PWM 18
#define RL_IN1 2
#define RL_IN2 4
#define RL_ENC_A 33
#define RL_ENC_B 32

// M3
#define FR_PWM 22
#define FR_IN1 14
#define FR_IN2 13
#define FR_ENC_A 36
#define FR_ENC_B 39

// ======================================================
// STRUKTUR DATA KOMUNIKASI
// ======================================================
typedef struct struct_message {
  int fl; int fr; int rl; int rr;
} struct_message;
struct_message incomingData;

// UBAH JADI FLOAT UNTUK RPM
typedef struct __attribute__((packed)) struct_feedback {
    float rpmRL; 
    float rpmRR; 
    float rpmFR;
} struct_feedback;
struct_feedback feedbackToMaster;

volatile unsigned long lastDataReceivedTime = 0;
const int MAX_SAFE_RPM = 500; 

SemaphoreHandle_t setpointMutex;

struct Motor {
  int pwmPin, in1, in2, encA, encB;
  volatile long encoderCount;
  volatile int lastEncoded;
  long lastCount;
  float ppr, setpoint, rpm, Kp, Ki, Kd, error, lastError, integral, derivative, control;
  int pwmOutput;
};

Motor RL = { RL_PWM, RL_IN1, RL_IN2, RL_ENC_A, RL_ENC_B, 0, 0, 0, 385,  0, 0, 0.8, 0.7, 0.02, 0, 0, 0, 0, 0, 0 };
Motor RR = { RR_PWM, RR_IN1, RR_IN2, RR_ENC_A, RR_ENC_B, 0, 0, 0, 385,  0, 0, 0.7, 0.6, 0.04, 0, 0, 0, 0, 0, 0 };
Motor FR = { FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B, 0, 0, 0, 385, 0, 0, 0.8, 0.6, 0.02, 0, 0, 0, 0, 0, 0 };

void updateEncoder(Motor &m) {
  int A = digitalRead(m.encA);
  int B = digitalRead(m.encB);
  int encoded = (A << 1) | B;
  int sum = (m.lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) m.encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) m.encoderCount--;
  m.lastEncoded = encoded;
}

void IRAM_ATTR encoderRL() { updateEncoder(RL); }
void IRAM_ATTR encoderRR() { updateEncoder(RR); }
void IRAM_ATTR encoderFR() { updateEncoder(FR); }

void putarMotor(Motor &m, int kecepatan) {
  int nilai_pwm = constrain(abs(kecepatan), 0, 255);
  if (kecepatan > 0) {
    digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, nilai_pwm);
  } else if (kecepatan < 0) {
    digitalWrite(m.in1, LOW); digitalWrite(m.in2, HIGH); analogWrite(m.pwmPin, nilai_pwm);
  } else {
    digitalWrite(m.in1, LOW); digitalWrite(m.in2, LOW); analogWrite(m.pwmPin, 0);
  }
}

void updateMotorPID(Motor &m, float dt) {
  long delta = m.encoderCount - m.lastCount;
  m.lastCount = m.encoderCount;
  float rpm_mentah = (delta * 600.0) / m.ppr; 
  m.rpm = (0.2 * rpm_mentah) + (0.8 * m.rpm); 

  if (m.setpoint == 0) {
    putarMotor(m, 0);
    m.integral = 0; m.lastError = 0;
    return; 
  }

  m.error = abs(m.setpoint) - abs(m.rpm);
  m.integral = constrain(m.integral + (m.error * dt), -100, 100); 
  m.derivative = (m.error - m.lastError) / dt;
  m.control = (m.Kp * m.error) + (m.Ki * m.integral) + (m.Kd * m.derivative);
  
  int MIN_PWM = 55; 
  float output_sementara = MIN_PWM + m.control; 
  if (output_sementara < (MIN_PWM - 5)) output_sementara = MIN_PWM - 5; 

  m.pwmOutput = constrain(output_sementara, 0, 255);
  if (m.setpoint > 0) putarMotor(m, m.pwmOutput);
  else if (m.setpoint < 0) putarMotor(m, -m.pwmOutput);
  m.lastError = m.error;
}

void TaskUARTReceive(void *pvParameters) {
  for (;;) {
    if (Serial2.available() >= sizeof(struct_message)) {
      struct_message tempMsg;
      Serial2.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

      if (abs(tempMsg.rl) <= MAX_SAFE_RPM && abs(tempMsg.rr) <= MAX_SAFE_RPM && abs(tempMsg.fr) <= MAX_SAFE_RPM) {
        if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
          RL.setpoint = tempMsg.rl;
          RR.setpoint = tempMsg.rr;
          FR.setpoint = tempMsg.fr;
          lastDataReceivedTime = millis(); 
          xSemaphoreGive(setpointMutex);
        }
      } else {
        while(Serial2.available() > 0) Serial2.read();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

void TaskMotorControl(void *pvParameters) {
  float dt = 0.1;
  const TickType_t xFrequency = pdMS_TO_TICKS(100); 
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(setpointMutex, portMAX_DELAY) == pdTRUE) {
      if (millis() - lastDataReceivedTime > 500) {
        RL.setpoint = 0; RR.setpoint = 0; FR.setpoint = 0;
      }
      updateMotorPID(RL, dt);
      updateMotorPID(RR, dt);
      updateMotorPID(FR, dt);
      xSemaphoreGive(setpointMutex);
    }

    // KIRIM RPM KE MASTER
    feedbackToMaster.rpmRL = RL.rpm; 
    feedbackToMaster.rpmRR = RR.rpm;
    feedbackToMaster.rpmFR = FR.rpm; 
    Serial2.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void setup() {
  pinMode(RL_PWM, OUTPUT); pinMode(RL_IN1, OUTPUT); pinMode(RL_IN2, OUTPUT);
  pinMode(RR_PWM, OUTPUT); pinMode(RR_IN1, OUTPUT); pinMode(RR_IN2, OUTPUT);
  pinMode(FR_PWM, OUTPUT); pinMode(FR_IN1, OUTPUT); pinMode(FR_IN2, OUTPUT);

  digitalWrite(RL_IN1, LOW); digitalWrite(RL_IN2, LOW); analogWrite(RL_PWM, 0);
  digitalWrite(RR_IN1, LOW); digitalWrite(RR_IN2, LOW); analogWrite(RR_PWM, 0);
  digitalWrite(FR_IN1, LOW); digitalWrite(FR_IN2, LOW); analogWrite(FR_PWM, 0);
  
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  while(Serial2.available() > 0) Serial2.read();

  setpointMutex = xSemaphoreCreateMutex();
  lastDataReceivedTime = millis(); 

  pinMode(RL_ENC_A, INPUT_PULLUP); pinMode(RL_ENC_B, INPUT_PULLUP);
  pinMode(RR_ENC_A, INPUT_PULLUP); pinMode(RR_ENC_B, INPUT_PULLUP);
  pinMode(FR_ENC_A, INPUT_PULLUP); pinMode(FR_ENC_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(RL_ENC_A), encoderRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RL_ENC_B), encoderRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RR_ENC_A), encoderRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RR_ENC_B), encoderRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);

  xTaskCreatePinnedToCore(TaskUARTReceive, "TaskUART", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskMotorControl, "TaskMotor", 4096, NULL, 2, NULL, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }