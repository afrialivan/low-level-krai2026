#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <ESP32Servo.h>  
#include <Arduino.h>

#define RX1_PIN 25
#define TX1_PIN 26

#define FL_PWM     22
#define FL_IN1     13
#define FL_IN2     14
#define FL_ENC_A   36
#define FL_ENC_B   39

#define SERVO1_PIN  21
#define SERVO2_PIN  15

#define M2_PWM      19  
#define M2_DIR      5   
#define M2_DIR2     23  

#define M3_PWM      18  
#define M3_DIR      2   
#define M3_DIR2     4   

#define PNEUMATIC_PIN -1

volatile unsigned long lastDataReceivedTime = 0;
const int MAX_SAFE_RPM = 500; 

typedef struct __attribute__((packed)) struct_message {
  float   data1; bool    data2; float   data3; float   data4; 
  bool    data5; bool    data6; int     data7; float   data8; int     data9;  
} struct_message;
struct_message incomingData;

// UBAH JADI 1 FLOAT UNTUK RPM FL
typedef struct __attribute__((packed)) struct_feedback {
  float rpmFL;
} struct_feedback;
struct_feedback feedbackToMaster;

SemaphoreHandle_t dataMutex;

struct Motor {
  int pwmPin, in1, in2, encA, encB;
  volatile long encoderCount;
  volatile int lastEncoded;
  long lastCount;
  float ppr, setpoint, rpm, Kp, Ki, Kd, error, lastError, integral, derivative, control;
  int pwmOutput;
};

Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B, 0, 0, 0, 385, 0, 0, 0.8, 0.6, 0.02, 0, 0, 0, 0, 0, 0 };

Servo servo1;
bool servoTriggered = false; 

void IRAM_ATTR encoderFL() {
  int A = digitalRead(FL.encA);
  int B = digitalRead(FL.encB);
  int encoded = (A << 1) | B;
  int sum = (FL.lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) FL.encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) FL.encoderCount--;
  FL.lastEncoded = encoded;
}

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
    if (Serial1.available() >= sizeof(struct_message)) {
      struct_message tempMsg;
      Serial1.readBytes((uint8_t*)&tempMsg, sizeof(struct_message));

      if (abs(tempMsg.data1) <= MAX_SAFE_RPM) {
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
          incomingData = tempMsg;
          FL.setpoint = incomingData.data1;
          lastDataReceivedTime = millis(); 
          xSemaphoreGive(dataMutex);
        }
      } else {
        while(Serial1.available() > 0) Serial1.read();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void TaskMotorAndFeedback(void *pvParameters) {
  float dt = 0.1;
  const TickType_t xFrequency = pdMS_TO_TICKS(100);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  int m2_state = 0, m3_state = 0;
  const int SPEED_PWM_CLIMB = 255; 
  const int SPEED_PWM_MOTOR_CLIMB = 100; 

  for (;;) {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      if (millis() - lastDataReceivedTime > 500) {
        FL.setpoint = 0; incomingData.data3 = 0; incomingData.data4 = 0;
      }
      updateMotorPID(FL, dt);
      m2_state = (int)incomingData.data3;
      m3_state = (int)incomingData.data4;
      xSemaphoreGive(dataMutex);
    }

    if (m2_state == 2) { digitalWrite(M2_DIR, HIGH); digitalWrite(M2_DIR2, LOW); analogWrite(M2_PWM, SPEED_PWM_CLIMB); }
    else if (m2_state == 1) { digitalWrite(M2_DIR, LOW); digitalWrite(M2_DIR2, HIGH); analogWrite(M2_PWM, SPEED_PWM_CLIMB); }
    else { digitalWrite(M2_DIR, LOW); digitalWrite(M2_DIR2, LOW); analogWrite(M2_PWM, 0); }

    if (m3_state == 2) { digitalWrite(M3_DIR, HIGH); digitalWrite(M3_DIR2, LOW); analogWrite(M3_PWM, SPEED_PWM_MOTOR_CLIMB); }
    else if (m3_state == 1) { digitalWrite(M3_DIR, LOW); digitalWrite(M3_DIR2, HIGH); analogWrite(M3_PWM, SPEED_PWM_MOTOR_CLIMB); }
    else { digitalWrite(M3_DIR, LOW); digitalWrite(M3_DIR2, LOW); analogWrite(M3_PWM, 0); }

    // KIRIM RPM KE MASTER
    feedbackToMaster.rpmFL = FL.rpm;
    Serial1.write((uint8_t *)&feedbackToMaster, sizeof(feedbackToMaster));
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// ... [TaskServoControl & TaskPneumaticControl biarkan sama persis] ...
bool posisiCapit = false;
void TaskServoControl(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      bool triggerValue = incomingData.data2;
      bool capit = incomingData.data7;
      xSemaphoreGive(dataMutex);

      if (triggerValue == 1 && !servoTriggered) {
        vTaskDelay(pdMS_TO_TICKS(600));
        digitalWrite(SERVO2_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));
        servo1.write(180);
        servoTriggered = true; 
      }
      if(capit && !posisiCapit) {
        vTaskDelay(pdMS_TO_TICKS(500)); 
        digitalWrite(SERVO2_PIN, HIGH);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

void TaskPneumaticControl(void *pvParameters) {
  bool pneumaticState = false;
  for (;;) {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      pneumaticState = incomingData.data6; 
      xSemaphoreGive(dataMutex);
    }
    if (PNEUMATIC_PIN >= 0) {
      if (pneumaticState) { pinMode(PNEUMATIC_PIN, OUTPUT); digitalWrite(PNEUMATIC_PIN, LOW); }
      else { pinMode(PNEUMATIC_PIN, INPUT); }
    }
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  while(Serial1.available() > 0) Serial1.read();

  dataMutex = xSemaphoreCreateMutex();
  lastDataReceivedTime = millis(); 

  pinMode(FL.encA, INPUT_PULLUP); pinMode(FL.encB, INPUT_PULLUP);
  pinMode(FL.pwmPin, OUTPUT); pinMode(FL.in1, OUTPUT); pinMode(FL.in2, OUTPUT);
  digitalWrite(FL.in1, LOW); digitalWrite(FL.in2, LOW); analogWrite(FL.pwmPin, 0);
  attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

  pinMode(M2_PWM, OUTPUT); pinMode(M2_DIR, OUTPUT); pinMode(M2_DIR2, OUTPUT);
  pinMode(M3_PWM, OUTPUT); pinMode(M3_DIR, OUTPUT); pinMode(M3_DIR2, OUTPUT);
  digitalWrite(M2_DIR, LOW); digitalWrite(M2_DIR2, LOW); 
  digitalWrite(M3_DIR, LOW); digitalWrite(M3_DIR2, LOW);

  if (PNEUMATIC_PIN != -1) pinMode(PNEUMATIC_PIN, INPUT);

  servo1.attach(SERVO1_PIN); servo1.write(75); 
  pinMode(SERVO2_PIN, OUTPUT);
  digitalWrite(SERVO2_PIN, HIGH);

  xTaskCreatePinnedToCore(TaskUARTReceive, "UARTRecv", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskMotorAndFeedback, "MotorPID", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskServoControl, "ServoCtrl", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskPneumaticControl, "PneumaticCtrl", 2048, NULL, 1, NULL, 0);
}
void loop() { vTaskDelete(NULL); }