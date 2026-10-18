#include <MPU9250.h>
#include <Wire.h>

MPU9250 mpu;

const int IN1_FL = 22;  const int IN2_FL = 21;  const int PWM_FL = 23;
const int IN1_FR = 27; const int IN2_FR = 26; const int PWM_FR = 14;
const int IN1_RL = 25; const int IN2_RL = 33; const int PWM_RL = 32;
const int IN1_RR = 19; const int IN2_RR = 18; const int PWM_RR = 32;

const int PIN_LED = 2;


void putarMotor(int in1, int in2, int pwm_pin, int speed) {
  int pwm_val = constrain(abs(speed), 0, 255);
  if (speed > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW); analogWrite(pwm_pin, pwm_val);
  } else if (speed < 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH); analogWrite(pwm_pin, pwm_val);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwm_pin, 0);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL untuk ESP32
  
  pinMode(IN1_FL, OUTPUT); pinMode(IN2_FL, OUTPUT); pinMode(PWM_FL, OUTPUT);
  pinMode(IN1_FR, OUTPUT); pinMode(IN2_FR, OUTPUT); pinMode(PWM_FR, OUTPUT);
  pinMode(IN1_RL, OUTPUT); pinMode(IN2_RL, OUTPUT); pinMode(PWM_RL, OUTPUT);
  pinMode(IN1_RR, OUTPUT); pinMode(IN2_RR, OUTPUT); pinMode(PWM_RR, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  if (!mpu.setup(0x68)) {
    while (1) { Serial.println("MPU Fail"); delay(500); }
  }
}

void loop() {
  if (mpu.update()) {
    static uint32_t prev_ms = millis();
    if (millis() - prev_ms > 50) {
      Serial.print("IMU,");
      Serial.println(mpu.getYaw());
      prev_ms = millis();
    }
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    int values[5];
    int count = 0;
    int startIdx = 0;

    for (int i = 0; i < data.length(); i++) {
      if (data.charAt(i) == ',' || i == data.length() - 1) {
        String res = (i == data.length() - 1) ? data.substring(startIdx) : data.substring(startIdx, i);
        values[count++] = res.toInt();
        startIdx = i + 1;
      }
      if (count >= 5) break;
    }

    if (count == 5) {
      putarMotor(IN1_FL, IN2_FL, PWM_FL, values[0]);
      putarMotor(IN1_FR, IN2_FR, PWM_FR, values[1]);
      putarMotor(IN1_RL, IN2_RL, PWM_RL, values[2]);
      putarMotor(IN1_RR, IN2_RR, PWM_RR, values[3]);
      digitalWrite(PIN_LED, values[4] == 1 ? HIGH : LOW);
    }
  }
}
