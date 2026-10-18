// ======================================================
// PIN MOTOR + ENCODER
// ======================================================

// ===== RL =====
#define RL_PWM   18
#define RL_IN1   4
#define RL_IN2   5
#define RL_ENC_A 35
#define RL_ENC_B 34

// ===== RR =====
#define RR_PWM   19
#define RR_IN1   2
#define RR_IN2   23
#define RR_ENC_A 39
#define RR_ENC_B 36

// ===== FR =====
#define FR_PWM   21
#define FR_IN1   26
#define FR_IN2   27
#define FR_ENC_A 25
#define FR_ENC_B 15

// ===== FL =====
#define FL_PWM   22
#define FL_IN1   13
#define FL_IN2   14
#define FL_ENC_A 33
#define FL_ENC_B 32

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

// ======================================================
// KONFIGURASI MOTOR
// ======================================================

Motor RL = {
  RL_PWM, RL_IN1, RL_IN2,
  RL_ENC_A, RL_ENC_B,
  0, 0,
  0,
  1420,
  0,
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
  0,
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
  40600,
  0,
  0,
  0.1, 8.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

Motor FL = {
  FL_PWM, FL_IN1, FL_IN2,
  FL_ENC_A, FL_ENC_B,
  0, 0,
  0,
  1420,
  0,
  0,
  0.1, 8.5, 0.03,
  0, 0, 0, 0,
  0,
  0
};

// ======================================================
// VARIABEL SERIAL
// ======================================================

int values[4];

// ======================================================
// ENCODER ISR
// ======================================================

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

// ======================================================
// ISR TIAP MOTOR
// ======================================================

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

// ======================================================
// FUNGSI PUTAR MOTOR
// ======================================================

void putarMotor(Motor &m, int kecepatan) {

  int nilai_pwm = abs(kecepatan);

  if (nilai_pwm > 255)
    nilai_pwm = 255;

  if (kecepatan > 0) {

    digitalWrite(m.in1, HIGH);
    digitalWrite(m.in2, LOW);

    analogWrite(m.pwmPin, nilai_pwm);

  } else if (kecepatan < 0) {

    digitalWrite(m.in1, LOW);
    digitalWrite(m.in2, HIGH);

    analogWrite(m.pwmPin, nilai_pwm);

  } else {

    digitalWrite(m.in1, LOW);
    digitalWrite(m.in2, LOW);

    analogWrite(m.pwmPin, 0);
  }
}

// ======================================================
// PID MOTOR
// ======================================================

void updateMotorPID(Motor &m, float dt) {

  long delta = m.encoderCount - m.lastCount;

  m.lastCount = m.encoderCount;

  m.rpm = (delta * 600.0) / m.ppr;

  m.error = abs(m.setpoint) - abs(m.rpm);

  m.integral += m.error * dt;

  m.integral = constrain(m.integral, -500, 500);

  m.derivative = (m.error - m.lastError) / dt;

  m.control =
      (m.Kp * m.error) +
      (m.Ki * m.integral) +
      (m.Kd * m.derivative);

  m.pwmOutput = constrain(abs(m.control), 0, 255);

  if (m.setpoint > 0) {
    putarMotor(m, m.pwmOutput);
  }
  else if (m.setpoint < 0) {
    putarMotor(m, -m.pwmOutput);
  }
  else {
    putarMotor(m, 0);
  }

  m.lastError = m.error;
}

// ======================================================
// TIMER
// ======================================================

unsigned long lastTime = 0;
float dt = 0.1;

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);
  Serial2.begin(115200);

  Motor* motors[] = {&RL, &RR, &FR, &FL};

  for (int i = 0; i < 4; i++) {

    pinMode(motors[i]->encA, INPUT);
    pinMode(motors[i]->encB, INPUT);

    pinMode(motors[i]->pwmPin, OUTPUT);

    pinMode(motors[i]->in1, OUTPUT);
    pinMode(motors[i]->in2, OUTPUT);
  }

  // INTERRUPT ENCODER

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

// ======================================================
// LOOP
// ======================================================

void loop() {

  // ==========================================
  // TERIMA DATA SERIAL
  // FORMAT:
  // FL,FR,RL,RR
  // ==========================================

  if (Serial.available() > 0) {

    String data = Serial.readStringUntil('\n');

    int index = 0;
    int start = 0;

    int end = data.indexOf(',');

    while (end != -1 && index < 4) {

      values[index++] = data.substring(start, end).toFloat();

      start = end + 1;

      end = data.indexOf(',', start);
    }

    if (index < 4) {
      values[index++] = data.substring(start).toFloat();
    }

    // ==========================================
    // MASUKKAN KE SETPOINT MASING MASING MOTOR
    // ==========================================

    if (index == 4) {

      FL.setpoint = values[0];
      FR.setpoint = values[1];
      RL.setpoint = values[2];
      RR.setpoint = values[3];
    }
  }

  // ==========================================
  // UPDATE PID
  // ==========================================

  unsigned long now = millis();

  if (now - lastTime >= 100) {

    updateMotorPID(FL, dt);
    updateMotorPID(FR, dt);
    updateMotorPID(RL, dt);
    updateMotorPID(RR, dt);

    Serial2.print("FL SP:");
    Serial2.print(FL.setpoint);
    Serial2.print(" RPM:");
    Serial2.print(FL.rpm);
    Serial2.print(" PWM:");
    Serial2.print(FL.pwmOutput);

    Serial2.print(" | FR SP:");
    Serial2.print(FR.setpoint);
    Serial2.print(" RPM:");
    Serial2.print(FR.rpm);
    Serial2.print(" PWM:");
    Serial2.print(FR.pwmOutput);

    Serial2.print(" | RL SP:");
    Serial2.print(RL.setpoint);
    Serial2.print(" RPM:");
    Serial2.print(RL.rpm);
    Serial2.print(" PWM:");
    Serial2.print(RL.pwmOutput);

    Serial2.print(" | RR SP:");
    Serial2.print(RR.setpoint);
    Serial2.print(" RPM:");
    Serial2.print(RR.rpm);
    Serial2.print(" PWM:");
    Serial2.println(RR.pwmOutput);

    lastTime = now;
  }
}