#include <esp_now.h>
#include <WiFi.h>

#define RL_PWM 18
#define RL_IN1 4
#define RL_IN2 5
#define RL_ENC_A 34
#define RL_ENC_B 35

// fl
#define RR_PWM 19
#define RR_IN1 2
#define RR_IN2 23
#define RR_ENC_A 39
#define RR_ENC_B 36

#define FR_PWM 21
#define FR_IN1 27
#define FR_IN2 26
#define FR_ENC_A 25
#define FR_ENC_B 15

#define FL_PWM 22
#define FL_IN1 14
#define FL_IN2 13
#define FL_ENC_A 32
#define FL_ENC_B 33

// ======================================================
// STRUKTUR DATA ESP-NOW
// ======================================================
uint8_t masterAddress[] = {0xD4, 0xE9, 0xF4, 0xA3, 0xA3, 0xE4};


typedef struct struct_message {
  int fl;
  int fr;
  int rl;
  int rr;
} struct_message;

struct_message incomingData;

typedef struct struct_feedback {
    float rpmFL; float rpmFR; float rpmRL; float rpmRR;
    long countFL; long countFR; long countRL; long countRR;
} struct_feedback;

struct_feedback feedbackToMaster;

// ======================================================
// STRUKTUR MOTOR
// ======================================================
struct Motor {
  int pwmPin;
  int in1;
  int in2;
  int encA;
  int encB;
  volatile long encoderCount;
  volatile int lastEncoded;
  long lastCount;
  float ppr;
  float setpoint;
  float rpm;
  float Kp;
  float Ki;
  float Kd;
  float error;
  float lastError;
  float integral;
  float derivative;
  float control;
  int pwmOutput;
};

// Inisialisasi Objek Motor
Motor RL = { RL_PWM, RL_IN1, RL_IN2, RL_ENC_A, RL_ENC_B,
             0, 0, 0,
             1420,  // ppr
             0,     // setpoint
             0,     // rpm
             2.1,   // kp
             2.5,   // ki
             0.03,  // kd
             0, 0, 0, 0, 0, 0 };
Motor RR = { RR_PWM, RR_IN1, RR_IN2, RR_ENC_A, RR_ENC_B,
             0, 0, 0,
             1420,  // ppr
             0,     // setpoint
             0,     // rpm
             2.1,   // kp
             2.5,   // ki
             0.03,  // kd
             0, 0, 0, 0, 0, 0 };
Motor FR = { FR_PWM, FR_IN1, FR_IN2, FR_ENC_A, FR_ENC_B,
             0, 0, 0,
             40600,  // ppr
             0,      // setpoint
             0,      // rpm
             2.1,    // kp
             2.5,    // ki
             0.03,   // kd
             0, 0, 0, 0, 0, 0 };
Motor FL = { FL_PWM, FL_IN1, FL_IN2, FL_ENC_A, FL_ENC_B,
             0, 0, 0,
             1420,  // ppr
             0,     // setpoint
             0,     // rpm
             2.1,   // kp
             2.5,   // ki
             0.03,  // kd
             0, 0, 0, 0, 0, 0 };

// ISR Handlers
void IRAM_ATTR encoderRL() { updateEncoder(RL); }
void IRAM_ATTR encoderRR() { updateEncoder(RR); }
void IRAM_ATTR encoderFR() { updateEncoder(FR); }
void IRAM_ATTR encoderFL() { updateEncoder(FL); }

void updateEncoder(Motor &m) {
  int A = digitalRead(m.encA);
  int B = digitalRead(m.encB);
  int encoded = (A << 1) | B;
  int sum = (m.lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) m.encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) m.encoderCount--;
  m.lastEncoded = encoded;
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
  m.rpm = (delta * 600.0) / m.ppr; 

  m.error = abs(m.setpoint) - abs(m.rpm);
  m.integral = constrain(m.integral + (m.error * dt), -500, 500);
  m.derivative = (m.error - m.lastError) / dt;
  m.control = (m.Kp * m.error) + (m.Ki * m.integral) + (m.Kd * m.derivative);
  m.pwmOutput = constrain(abs(m.control), 0, 255);

  if (m.setpoint > 0) putarMotor(m, m.pwmOutput);
  else if (m.setpoint < 0) putarMotor(m, -m.pwmOutput);
  else {
    putarMotor(m, 0);
    m.integral = 0;
  }
  m.lastError = m.error;
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingRawData, int len) {
  memcpy(&incomingData, incomingRawData, sizeof(incomingData));
  FL.setpoint = incomingData.fl;
  FR.setpoint = incomingData.fr;
  RL.setpoint = incomingData.rl;
  RR.setpoint = incomingData.rr;
}

unsigned long lastTime = 0;
float dt = 0.1;

void setup() {
  Serial.begin(115200);

  // Setup Pins
  Motor *motors[] = { &RL, &RR, &FR, &FL };
  for (int i = 0; i < 4; i++) {
    pinMode(motors[i]->encA, INPUT);
    pinMode(motors[i]->encB, INPUT);
    pinMode(motors[i]->pwmPin, OUTPUT);
    pinMode(motors[i]->in1, OUTPUT);
    pinMode(motors[i]->in2, OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(RL_ENC_A), encoderRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RL_ENC_B), encoderRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RR_ENC_A), encoderRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RR_ENC_B), encoderRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

  // WAJIB: Inisialisasi WiFi dan ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW Init");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  // Daftarkan Master sebagai Peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Gagal tambah Peer");
    return;
  }

  lastTime = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastTime >= 100) {
    updateMotorPID(FL, dt);
    updateMotorPID(FR, dt);
    updateMotorPID(RL, dt);
    updateMotorPID(RR, dt);

    // Siapkan Feedback
    feedbackToMaster.rpmFL = FL.rpm; feedbackToMaster.rpmFR = FR.rpm;
    feedbackToMaster.rpmRL = RL.rpm; feedbackToMaster.rpmRR = RR.rpm;
    feedbackToMaster.countFL = FL.encoderCount; feedbackToMaster.countFR = FR.encoderCount;
    feedbackToMaster.countRL = RL.encoderCount; feedbackToMaster.countRR = RR.encoderCount;

    // Kirim ke Master
    esp_now_send(masterAddress, (uint8_t *) &feedbackToMaster, sizeof(feedbackToMaster));

    // Debugging (pindahkan ke sini agar tidak membebani PID)
    Serial.printf("RPM FR: %.2f | FL: %.2f\n | RR: %.2f | RL: %.2f\n", FR.setpoint, FL.setpoint, RR.setpoint, RL.setpoint);

    lastTime = now;
  }
}