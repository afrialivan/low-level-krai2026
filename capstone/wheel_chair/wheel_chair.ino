const int L_IN1 = 8;
const int L_IN2 = 9;
const int L_PWM = 10; 


const int R_IN3 = 4;
const int R_IN4 = 5;
const int R_PWM = 6;  

void setup() {
  
  Serial.begin(9600);
  
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  
  pinMode(R_IN3, OUTPUT);
  pinMode(R_IN4, OUTPUT);
  pinMode(R_PWM, OUTPUT);
}

void loop() {
  
  if (Serial.available() > 0) {
    // Baca string masuk sampai menemukan karakter newline (\n)
    String data = Serial.readStringUntil('\n');
    
    
    int commaIndex = data.indexOf(',');
    
    if (commaIndex > 0) {
      
      float speed_l = data.substring(0, commaIndex).toFloat();
      float speed_r = data.substring(commaIndex + 1).toFloat();
      
      
      setMotor(L_IN1, L_IN2, L_PWM, speed_l); 
      setMotor(R_IN3, R_IN4, R_PWM, speed_r); 
    }
  }
}

void setMotor(int pin_in1, int pin_in2, int pin_pwm, float speed) {
  int pwm_val = constrain(abs(speed) * 255, 0, 255);
  
  if (speed > 0) { 
    digitalWrite(pin_in1, HIGH);
    digitalWrite(pin_in2, LOW);
  } else if (speed < 0) { 
    digitalWrite(pin_in1, LOW);
    digitalWrite(pin_in2, HIGH);
  } else { // 
    digitalWrite(pin_in1, LOW);
    digitalWrite(pin_in2, LOW);
  }
  analogWrite(pin_pwm, pwm_val);
}