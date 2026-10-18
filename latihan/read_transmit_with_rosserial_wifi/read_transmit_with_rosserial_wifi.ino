#include <WiFi.h>
#include <ros.h>
#include <std_msgs/Float32.h>

// ===== WIFI =====
const char* ssid = "kct";
const char* password = "KCTunhas2026";

IPAddress server(10,30,63,114);
uint16_t serverPort = 11411;

// ===== ROS =====
ros::NodeHandle nh;

std_msgs::Float32 rpm_msg;
ros::Publisher pub_rpm("rpm", &rpm_msg);

// ===== TIMER =====
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  nh.advertise(pub_rpm);
}

void loop() {
  nh.spinOnce();

  if (millis() - lastTime > 200) { // 5 Hz
    rpm_msg.data = 123.4;
    pub_rpm.publish(&rpm_msg);

    Serial.println("Kirim data dummy");

    lastTime = millis();
  }

  delay(1);
}
