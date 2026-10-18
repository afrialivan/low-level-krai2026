const int sensorPin = 27;

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
}

void loop() {
  int statusSensor = digitalRead(sensorPin);

  if (statusSensor == LOW) {
    Serial.println("Area Bersih.");
  } else {
    Serial.println("Ada Objek Terdeteksi!");
  }
  
  delay(100);
}