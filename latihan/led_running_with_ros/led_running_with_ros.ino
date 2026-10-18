int led1 = 13;
int led2 = 12;

String data = "";

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    data += c;
  }

  if (data.length() >= 2) {
    if (data[0] == '1') digitalWrite(led1, HIGH);
    else digitalWrite(led1, LOW);

    if (data[1] == '1') digitalWrite(led2, HIGH);
    else digitalWrite(led2, LOW);

    data = ""; // reset
  }
}
