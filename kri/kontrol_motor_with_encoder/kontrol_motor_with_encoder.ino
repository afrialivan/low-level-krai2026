// ===== RL =====
#define RL_PWM   18
#define RL_IN1   5
#define RL_IN2   4
#define RL_ENC_A 35
#define RL_ENC_B 34

// ===== RR =====
#define RR_PWM   19
#define RR_IN1   2
#define RR_IN2   23
#define RR_ENC_A 39
#define RR_ENC_B 36

// // ===== FR =====
// #define FR_PWM   21
// #define FR_IN1   26
// #define FR_IN2   27
// #define FR_ENC_A 25
// #define FR_ENC_B 15

// // ===== FL =====
// #define FL_PWM   22
// #define FL_IN1   14
// #define FL_IN2   13
// #define FL_ENC_A 33
// #define FL_ENC_B 32

// ==================
// MOTOR DEPAN
// ==================

// ===== FR =====
#define FR_PWM   18
#define FR_IN1   4
#define FR_IN2   2
#define FR_ENC_A 34
#define FR_ENC_B 35

// ===== FL =====
#define FL_PWM   19
#define FL_IN1   5
#define FL_IN2   23
#define FL_ENC_A 32
#define FL_ENC_B 33

// =========================
// Struktur Motor
// =========================
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

// =========================
// Konfigurasi Motor
// =========================

Motor RL = {
  RL_PWM, RL_IN1, RL_IN2,
  RL_ENC_A, RL_ENC_B,
  0, 0,
  0,
  1001,
  40.0,
  0,
  0.1, 8.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

Motor RR = {
  RR_PWM, RR_IN1, RR_IN2,
  RR_ENC_A, RR_ENC_B,
  0, 0,
  0,
  1420,
  40.0,
  0,
  0.1, 8.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

Motor FR = {
  FR_PWM, FR_IN1, FR_IN2,
  FR_ENC_A, FR_ENC_B,
  0, 0,
  0,
  1001,
  40.0,
  0,
  1.0, 3.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

Motor FL = {
  FL_PWM, FL_IN1, FL_IN2,
  FL_ENC_A, FL_ENC_B,
  0, 0,
  0,
  1001,
  40.0,
  0,
  2.1, 2.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

// =========================
// ISR Encoder
// =========================

void updateEncoder(Motor &m) {
  int A = digitalRead(m.encA);
  int B = digitalRead(m.encB);

  int encoded = (A << 1) | B;
  int sum = (m.lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    m.encoderCount++;

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    m.encoderCount--;

  m.lastEncoded = encoded;
}

// ISR tiap motor
void IRAM_ATTR encoderRL() {
  updateEncoder(RL);
}

void IRAM_ATTR encoderRR() {
  updateEncoder(RR);
}

void IRAM_ATTR encoderFR() {
  updateEncoder(FR);
}

void IRAM_ATTR encoderFL() {
  updateEncoder(FL);
}

// =========================
// PID Motor
// =========================

void updateMotorPID(Motor &m, float dt) {

  long delta = m.encoderCount - m.lastCount;
  m.lastCount = m.encoderCount;

  m.rpm = (delta * 600.0) / m.ppr;

  m.error = m.setpoint - m.rpm;

  m.integral += m.error * dt;
  m.integral = constrain(m.integral, -500, 500);

  m.derivative = (m.error - m.lastError) / dt;

  m.control =
      (m.Kp * m.error) +
      (m.Ki * m.integral) +
      (m.Kd * m.derivative);

  m.pwmOutput = constrain(m.control, 0, 255);

  analogWrite(m.pwmPin, m.pwmOutput);

  m.lastError = m.error;
}

// =========================
// Setup
// =========================

unsigned long lastTime = 0;
float dt = 0.1;

void setup() {

  Serial.begin(115200);

  Motor* motors[] = {&RL, &RR, &FR, &FL};

  for (int i = 0; i < 4; i++) {

    pinMode(motors[i]->encA, INPUT);
    pinMode(motors[i]->encB, INPUT);

    pinMode(motors[i]->pwmPin, OUTPUT);

    pinMode(motors[i]->in1, OUTPUT);
    pinMode(motors[i]->in2, OUTPUT);

    digitalWrite(motors[i]->in1, HIGH);
    digitalWrite(motors[i]->in2, LOW);
  }

  // Interrupt Encoder
  attachInterrupt(digitalPinToInterrupt(RL_ENC_A), encoderRL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RL_ENC_B), encoderRL, CHANGE);

  attachInterrupt(digitalPinToInterrupt(RR_ENC_A), encoderRR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RR_ENC_B), encoderRR, CHANGE);

  attachInterrupt(digitalPinToInterrupt(FR_ENC_A), encoderFR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR_ENC_B), encoderFR, CHANGE);

  attachInterrupt(digitalPinToInterrupt(FL_ENC_A), encoderFL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FL_ENC_B), encoderFL, CHANGE);

  lastTime = millis();
}

// =========================
// Loop
// =========================

void loop() {

  unsigned long now = millis();

  if (now - lastTime >= 100) {

    updateMotorPID(RL, dt);
    updateMotorPID(RR, dt);
    updateMotorPID(FR, dt);
    updateMotorPID(FL, dt);

    // Serial.print("RL RPM:");
    // Serial.print(RL.rpm);
    // Serial.print(" PWM:");
    // Serial.print(RL.pwmOutput);

    // Serial.print(" | RR RPM:");
    // Serial.print(RR.rpm);
    // Serial.print(" PWM:");
    // Serial.print(RR.pwmOutput);

    // Serial.print(" | FR RPM:");
    Serial.print("RPM:");
    Serial.print(FL.rpm);
    Serial.print("PWM:");
    Serial.println(FL.pwmOutput);

    // Serial.print(" | FL RPM:");
    // Serial.print(FL.rpm);
    // Serial.print(" PWM:");
    // Serial.println(FL.pwmOutput);

    lastTime = now;
  }
}