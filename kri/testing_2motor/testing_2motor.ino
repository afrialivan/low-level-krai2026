
// FR
const int pwm3 = 18;
const int in1_M3 = 2;
const int in2_M3 = 4;

// FL
const int pwm4 = 19;
const int in1_M4 = 5;
const int in2_M4 = 23;

void setup() {
  pinMode(pwm3, OUTPUT);
  pinMode(in1_M3, OUTPUT);
  pinMode(in2_M3, OUTPUT);

  pinMode(pwm4, OUTPUT);
  pinMode(in1_M4, OUTPUT);
  pinMode(in2_M4, OUTPUT);
}

void loop() {
  // fr
  analogWrite(pwm3, 255); 
  digitalWrite(in1_M3, LOW);  
  digitalWrite(in2_M3, HIGH); 
  
  // fl
  analogWrite(pwm4, 255); 
  digitalWrite(in1_M4, LOW);  
  digitalWrite(in2_M4, HIGH); 
}