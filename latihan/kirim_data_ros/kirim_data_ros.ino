float suhu = 25.5;
float kelembaban = 60.2;
int jarak = 120;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print(suhu);
  Serial.print(",");
  Serial.print(kelembaban);
  Serial.print(",");
  Serial.println(jarak);

  delay(1000);
}
