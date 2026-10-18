#define ENCODER_A 33
#define ENCODER_B 32
#define PPR 385

//#define motorPWMPin 18
//#define motorDirPin1 23
//#define motorDirPin2 5

#define motorPWMPin 18
#define motorDirPin1 4
#define motorDirPin2 2

volatile long encoderCount = 0;
volatile int lastEncoded = 0;
long lastCount = 0;

float setpoint = 300.0;   // RPM
float Kp = 0.1;
float Ki = 6.5;
float Kd = 0.03;

float error, lastError = 0;
float integral = 0;
float derivative;
float control;

unsigned long lastTime = 0;
float dt = 0.1;  // 100 ms

float rpm = 0;
int pwmOutput = 0;

void IRAM_ATTR encoderISR() {
  int A = digitalRead(ENCODER_A);
  int B = digitalRead(ENCODER_B);

  int encoded = (A << 1) | B;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderCount++;

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderCount--;

  lastEncoded = encoded;
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);

  pinMode(motorPWMPin, OUTPUT);
  pinMode(motorDirPin1, OUTPUT);
  pinMode(motorDirPin2, OUTPUT);

  digitalWrite(motorDirPin1, HIGH);
  digitalWrite(motorDirPin2, LOW);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);

  lastTime = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastTime >= 100) {
    long delta = encoderCount - lastCount;
    lastCount = encoderCount;

    rpm = (delta * 600.0) / PPR;

    error = setpoint - rpm;
    integral += error * dt;
    integral = constrain(integral, -500, 500);
    derivative = (error - lastError) / dt;

    control = (Kp * error) + (Ki * integral) + (Kd * derivative);

    pwmOutput = constrain(control, 0, 255);
    analogWrite(motorPWMPin, pwmOutput);

    lastError = error;
    lastTime = now;

    Serial.print("RPM:");
    Serial.print(rpm);
    Serial.print(" Setpoint:");
    Serial.print(setpoint);
    Serial.print(" PWM:");
    Serial.println(pwmOutput);
  }
}
