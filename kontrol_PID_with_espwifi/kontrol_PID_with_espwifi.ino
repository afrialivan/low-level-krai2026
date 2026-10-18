#include <WiFi.h>
#include <ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Float32MultiArray.h>

// ===== WIFI =====
const char* ssid = "kct";
const char* password = "KCTunhas2026";

IPAddress server(192,168,8,10);
uint16_t serverPort = 11411;

// ===== ROS =====
ros::NodeHandle nh;

// publish RPM ke ROS
std_msgs::Float32 rpm_msg;
ros::Publisher pub_rpm("rpm", &rpm_msg);

// subscribe setpoint + PID
float setpoint = 0;
float Kp = 0.0;
float Ki = 0.0;
float Kd = 0.0;

void pidCallback(const std_msgs::Float32MultiArray& msg) {
  if (msg.data_length >= 4) {
    setpoint = msg.data[0];
    Kp = msg.data[1];
    Ki = msg.data[2];
    Kd = msg.data[3];
  }
}

ros::Subscriber<std_msgs::Float32MultiArray> sub_pid("pid_values", pidCallback);

// ===== ENCODER =====
#define ENCODER_A 32
#define ENCODER_B 33
#define PPR 1001

volatile long encoderCount = 0;
volatile int lastEncoded = 0;
long lastCount = 0;

// ===== MOTOR =====
#define motorPWMPin 21
#define motorDirPin1 13
#define motorDirPin2 14

// ===== PID =====
float error, lastError = 0;
float integral = 0;
float derivative;
float control;

float rpm = 0;
int pwmOutput = 0;

unsigned long lastTime = 0;
float dt = 0.1; // 100 ms

// ===== ENCODER ISR =====
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

  // ===== WIFI =====
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  // ===== ROS =====
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  nh.advertise(pub_rpm);
  nh.subscribe(sub_pid);

  // ===== MOTOR =====
  pinMode(motorPWMPin, OUTPUT);
  pinMode(motorDirPin1, OUTPUT);
  pinMode(motorDirPin2, OUTPUT);

  digitalWrite(motorDirPin1, HIGH);
  digitalWrite(motorDirPin2, LOW);

  // ===== ENCODER =====
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);

  lastTime = millis();
}

void loop() {
  nh.spinOnce();

  unsigned long now = millis();

  if (now - lastTime >= 100) {

    long delta = encoderCount - lastCount;
    lastCount = encoderCount;

    rpm = (delta * 600.0) / PPR;

    // ===== PID =====
    error = setpoint - rpm;

    integral += error * dt;
    integral = constrain(integral, -500, 500);

    derivative = (error - lastError) / dt;

    control = (Kp * error) + (Ki * integral) + (Kd * derivative);

    pwmOutput = constrain(control, 0, 255);
    analogWrite(motorPWMPin, pwmOutput);

    lastError = error;
    lastTime = now;

    // ===== KIRIM RPM KE ROS =====
    rpm_msg.data = rpm;
    pub_rpm.publish(&rpm_msg);

    // ===== DEBUG =====
    Serial.print("RPM:");
    Serial.print(rpm);
    Serial.print(" SP:");
    Serial.print(setpoint);
    Serial.print(" PWM:");
    Serial.println(pwmOutput);
  }

  delay(1);
}
