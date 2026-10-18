#include <ros.h>
#include <std_msgs/String.h>

// Inisialisasi Node ROS
ros::NodeHandle nh;

// Inisialisasi pesan dan Publisher
std_msgs::String str_msg;
ros::Publisher chatter("sensor_data", &str_msg);

void setup() {
  nh.initNode();
  nh.advertise(chatter);
}

void loop() {
  str_msg.data = "Halo dari ESP32!";
  chatter.publish(&str_msg);
  
  nh.spinOnce();
  delay(1000); // Publish setiap 1 detik
}