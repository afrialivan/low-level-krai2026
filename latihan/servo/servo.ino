#include <ESP32Servo.h>

// pwm = kuning, vcc = merah, gnd = coklat
Servo myservo;
int servoPin = 21; 

void setup() {ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);           // Standar 50Hz
  myservo.attach(servoPin, 500, 2400);  // Pin, Pulse lebar min, Pulse lebar max
  myservo.write(0);
  Serial.begin(115200);
}

void loop() {
  myservo.write(0); // menutup
  delay(1000); 
  
  myservo.write(180); // membuka
  delay(1000);

  Serial.println("tes");

}