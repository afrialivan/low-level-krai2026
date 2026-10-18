// FL
const int pwm1 = 18;
const int in1_M1 = 5;
const int in2_M1 = 4;

// FR
const int pwm2 = 19;
const int in1_M2 = 23;
const int in2_M2 = 2;

// RR
const int pwm3 = 21;
const int in1_M3 = 26;
const int in2_M3 = 27;

// RL
const int pwm4 = 22;
const int in1_M4 = 13;
const int in2_M4 = 14;

void setup() {
  Serial.begin(115200);
  pinMode(pwm1, OUTPUT);
  pinMode(in1_M1, OUTPUT);
  pinMode(in2_M1, OUTPUT);

  pinMode(pwm2, OUTPUT);
  pinMode(in1_M2, OUTPUT);
  pinMode(in2_M2, OUTPUT);

  pinMode(pwm3, OUTPUT);
  pinMode(in1_M3, OUTPUT);
  pinMode(in2_M3, OUTPUT);

  pinMode(pwm4, OUTPUT);
  pinMode(in1_M4, OUTPUT);
  pinMode(in2_M4, OUTPUT);
}

void loop() {
  analogWrite(pwm1, 255); 
  digitalWrite(in1_M1, LOW);  
  digitalWrite(in2_M1, HIGH);

  analogWrite(pwm2, 255); 
  digitalWrite(in1_M2, LOW);  
  digitalWrite(in2_M2, HIGH);

  analogWrite(pwm3, 255); 
  digitalWrite(in1_M3, LOW);  
  digitalWrite(in2_M3, HIGH);

  analogWrite(pwm4, 255); 
  digitalWrite(in1_M4, LOW);  
  digitalWrite(in2_M4, HIGH);
}