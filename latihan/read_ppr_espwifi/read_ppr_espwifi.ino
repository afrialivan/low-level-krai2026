#include <WiFi.h>
#include <ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int32.h>

// ===== WIFI =====
const char* ssid = "testing";
const char* password = "44444444";

IPAddress server(10,30,63,114);
uint16_t serverPort = 11411;

// ===== ROS =====
ros::NodeHandle nh;

// rpm
std_msgs::Float32 rpm_msg;
ros::Publisher pub_rpm("rpm", &rpm_msg);

// ppr
std_msgs::Int32 count_msg;
ros::Publisher pub_count("count", &count_msg);

// ===== ENCODER =====
#define ENCODER_A 32
#define ENCODER_B 33
#define PPR 1430

volatile long encoderCount = 0;
volatile int lastEncoded = 0;
long lastCount = 0;

// ===== TIMER =====
unsigned long lastTime = 0;

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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  // ===== ROS =====
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  nh.advertise(pub_rpm);
  nh.advertise(pub_count);

  // ===== ENCODER =====
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoderISR, CHANGE);
}

void loop() {
  nh.spinOnce();

  if (millis() - lastTime > 100) {

    long countNow = encoderCount;
    long delta = countNow - lastCount;
    lastCount = countNow;

    float rpm = (delta * 600.0) / PPR;

    // ===== publish RPM =====
    rpm_msg.data = rpm;
    pub_rpm.publish(&rpm_msg);

    // ===== publish COUNT =====
    count_msg.data = countNow;
    pub_count.publish(&count_msg);

    // ===== debug serial =====
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | Count: ");
    Serial.println(countNow);

    lastTime = millis();
  }

  delay(1);
}
