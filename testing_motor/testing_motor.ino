// kiri belakang
const int pwm2 = 22;
const int in1_M2 = 13;
const int in2_M2 = 14;

// // kanan belakang
// const int pwm1 = 19;
// const int in1_M1 = 5;
// const int in2_M1 = 23;

// // kanan depan
// const int pwm4 = 22;
// const int in1_M4 = 14;
// const int in2_M4 = 13;

// // kiri depan
// const int pwm3 = 22;
// const int in1_M3 = 21;
// const int in2_M3 = 15;


void setup() {
  Serial.begin(115200);
  // pinMode(pwm1, OUTPUT);
  // pinMode(in1_M1, OUTPUT);
  // pinMode(in2_M1, OUTPUT);

  pinMode(pwm2, OUTPUT);
  pinMode(in1_M2, OUTPUT);
  pinMode(in2_M2, OUTPUT);

  // // pinMode(pwm3, OUTPUT);
  // // pinMode(in1_M3, OUTPUT);
  // // pinMode(in2_M3, OUTPUT);

  // pinMode(pwm4, OUTPUT);
  // pinMode(in1_M4, OUTPUT);
  // pinMode(in2_M4, OUTPUT);
}

void loop() {
  // analogWrite(pwm1, 255);
  analogWrite(pwm2, 255); 
  // analogWrite(pwm3, 255); 
  // analogWrite(pwm4, 255); 

  // digitalWrite(in1_M1, LOW);  
  // digitalWrite(in2_M1, HIGH);

  digitalWrite(in1_M2, LOW);  
  digitalWrite(in2_M2, HIGH);
  
  // // digitalWrite(in1_M3, LOW);  
  // // digitalWrite(in2_M3, HIGH);
  
  // digitalWrite(in1_M4, LOW);  
  // digitalWrite(in2_M4, HIGH);

  // delay(1000);
  // maju
  // digitalWrite(in1_M1, HIGH);  
  // digitalWrite(in2_M1, LOW);

  // digitalWrite(in1_M2, HIGH);  
  // digitalWrite(in2_M2, LOW);
  
  // digitalWrite(in1_M3, HIGH);  
  // digitalWrite(in2_M3, LOW);
  
  // digitalWrite(in1_M4, HIGH);  
  // digitalWrite(in2_M4, LOW);
  // delay(1000);
  
}