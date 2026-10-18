#include <ESP32Servo.h>

// PWM = Kuning/Oranye, VCC = Merah, GND = Coklat/Hitam
Servo myservo1;
Servo myservo2;

// Menentukan pin untuk masing-masing servo
int servoPin1 = 14; 
int servoPin2 = 12; // Pin baru untuk servo kedua

void setup() {
  // Mengalokasikan timer PWM ESP32
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1); // Alokasikan timer tambahan untuk servo kedua
  
  // Pengaturan Servo 1
  myservo1.setPeriodHertz(50);           // Standar 50Hz
  myservo1.attach(servoPin1, 500, 2400); // Pin, Pulse lebar min, Pulse lebar max
  
  // Pengaturan Servo 2
  myservo2.setPeriodHertz(50);           
  myservo2.attach(servoPin2, 500, 2400); 

  // Posisi awal (0 derajat)
  myservo1.write(180);
  myservo2.write(180);
  delay(1000);
}

void loop() {
  myservo1.write(85);
  myservo2.write(85);
}