#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MadgwickAHRS.h>

Adafruit_MPU6050 mpu;
Madgwick filter;

void setup() {
  Serial.begin(115200);

  if (!mpu.begin()) {
    Serial.println("MPU6050 tidak ditemukan");
    while (1);
  }

  filter.begin(100); // frekuensi 100 Hz
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // update filter
  filter.updateIMU(
    g.gyro.x, g.gyro.y, g.gyro.z,
    a.acceleration.x, a.acceleration.y, a.acceleration.z
  );

  // PERBAIKAN: Ambil quaternion langsung dari variabel public
  float q0 = filter.q0; // W
  float q1 = filter.q1; // X
  float q2 = filter.q2; // Y
  float q3 = filter.q3; // Z

  // kirim: ax,ay,az,gx,gy,gz,qx,qy,qz,qw
  Serial.print(a.acceleration.x); Serial.print(",");
  Serial.print(a.acceleration.y); Serial.print(",");
  Serial.print(a.acceleration.z); Serial.print(",");
  Serial.print(g.gyro.x); Serial.print(",");
  Serial.print(g.gyro.y); Serial.print(",");
  Serial.print(g.gyro.z); Serial.print(",");
  Serial.print(q1); Serial.print(","); // X
  Serial.print(q2); Serial.print(","); // Y
  Serial.print(q3); Serial.print(","); // Z
  Serial.println(q0);                  // W

  delay(10);
}
