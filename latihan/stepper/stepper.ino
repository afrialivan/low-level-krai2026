int en = 4, dir = 16, pul = 17;
int jeda = 1000;
void setup() {
  // put your setup code here, to run once:
  pinMode(en, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(pul, OUTPUT);
}

void loop() {
  digitalWrite(dir, HIGH);
  for(int i=0; i<200; i++) {
    digitalWrite(pul, HIGH);
    delayMicroseconds(jeda);
    digitalWrite(pul, LOW);
    delayMicroseconds(jeda);
  }
  delay(1000);
  
  // digitalWrite(dir, LOW);
  // for(int i=0; i<200; i++) {
  //   digitalWrite(pul, HIGH);
  //   delayMicroseconds(jeda);
  //   digitalWrite(pul, LOW);
  //   delayMicroseconds(jeda);
  // }
  // delay(1000);
  digitalWrite(en, HIGH); // matikan jika mau cari putar terus tanpa henti
}
