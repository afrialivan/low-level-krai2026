// ===================== WHEEL =====================
// W1 = depan kiri
const int W1_PWM = 18;
const int W1_IN1 = 23;
const int W1_IN2 = 5;

const int W2_PWM = 19;
const int W2_IN1 = 4;
const int W2_IN2 = 2;

const int W3_PWM = 21;
const int W3_IN1 = 13;
const int W3_IN2 = 14;

const int W4_PWM = 22;
const int W4_IN1 = 27;
const int W4_IN2 = 26;

void setup() {
  // W1
  pinMode(W1_PWM, OUTPUT);
  pinMode(W1_IN1, OUTPUT);
  pinMode(W1_IN2, OUTPUT);

  // W2
  pinMode(W2_PWM, OUTPUT);
  pinMode(W2_IN1, OUTPUT);
  pinMode(W2_IN2, OUTPUT);

  // W3
  pinMode(W3_PWM, OUTPUT);
  pinMode(W3_IN1, OUTPUT);   
  pinMode(W3_IN2, OUTPUT);

  // W4
  pinMode(W4_PWM, OUTPUT);
  pinMode(W4_IN1, OUTPUT);
  pinMode(W4_IN2, OUTPUT);
}

void loop() {

  // ===================== TEST W1 =====================
  digitalWrite(W1_IN1, HIGH);
  digitalWrite(W1_IN2, LOW);
  analogWrite(W1_PWM, 255);

 

  // ===================== TEST W2 =====================
  digitalWrite(W2_IN1, HIGH);
  digitalWrite(W2_IN2, LOW);
  analogWrite(W2_PWM, 255);
  
  

  // ===================== TEST W3 =====================
  digitalWrite(W3_IN1, HIGH);
  digitalWrite(W3_IN2, LOW);
  analogWrite(W3_PWM, 255);
 

  // ===================== TEST W4 =====================
  digitalWrite(W4_IN1, HIGH);
  digitalWrite(W4_IN2, LOW);
  analogWrite(W4_PWM, 255);
}
