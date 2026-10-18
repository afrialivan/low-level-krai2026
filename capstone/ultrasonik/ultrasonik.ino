#include <ros.h>
#include <std_msgs/Int32.h>

const int pinTrig = 9;
const int pinEcho = 11;

long durasi;
int jarak;

ros::NodeHandle nh;

std_msgs::Int32 distance_msg;
ros::Publisher pub_distance("ultrasonic_distance", &distance_msg);

void setup() {
  nh.initNode();
  nh.advertise(pub_distance);
  
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT); 
}

void loop() {

  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  
  durasi = pulseIn(pinEcho, HIGH);
  jarak = durasi * 0.034 / 2;
  
  distance_msg.data = jarak;
  
  pub_distance.publish(&distance_msg);
  
  nh.spinOnce();
  
  delay(200);
}