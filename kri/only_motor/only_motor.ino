
const int IN1_FL = 13;  const int IN2_FL = 14;  const int PWM_FL = 22;
const int IN1_FR = 26; const int IN2_FR = 27; const int PWM_FR = 21;
const int IN1_RL = 4; const int IN2_RL = 5; const int PWM_RL = 18;
const int IN1_RR = 2; const int IN2_RR = 23; const int PWM_RR = 19;

// const int PIN_LED = 15;

// Variabel Penampung PWM
int pwm_fl = 0, pwm_fr = 0, pwm_rl = 0, pwm_rr = 0, led_state = 0; // <-- PERBAIKAN: Tambah led_state
// int values[5]; // <-- PERBAIKAN: Ukuran array jadi 5 (4 motor + 1 LED)
int values[4];

void putarMotor(int in1, int in2, int pwm_pin, int kecepatan) {
  int nilai_pwm = abs(kecepatan);
  if(nilai_pwm > 255) nilai_pwm = 255; 

  if (kecepatan > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW); analogWrite(pwm_pin, nilai_pwm);
  } else if (kecepatan < 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH); analogWrite(pwm_pin, nilai_pwm);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwm_pin, 0);
  }
}

void setup() {
  pinMode(IN1_FL, OUTPUT); pinMode(IN2_FL, OUTPUT); pinMode(PWM_FL, OUTPUT);
  pinMode(IN1_FR, OUTPUT); pinMode(IN2_FR, OUTPUT); pinMode(PWM_FR, OUTPUT);
  pinMode(IN1_RL, OUTPUT); pinMode(IN2_RL, OUTPUT); pinMode(PWM_RL, OUTPUT);
  pinMode(IN1_RR, OUTPUT); pinMode(IN2_RR, OUTPUT); pinMode(PWM_RR, OUTPUT);
  
  Serial.begin(115200); 
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    int index = 0, start = 0;
    int end = data.indexOf(',');

    while (end != -1 && index < 5) { 
      values[index++] = data.substring(start, end).toInt();
      start = end + 1;
      end = data.indexOf(',', start);
    }
    if (index < 5) values[index++] = data.substring(start).toInt();

    if (index == 4) {
      pwm_fl = values[0];
      pwm_fr = values[1];
      pwm_rl = values[2];
      pwm_rr = values[3];
      // led_state = values[4];

      putarMotor(IN1_FL, IN2_FL, PWM_FL, pwm_fl);
      putarMotor(IN1_FR, IN2_FR, PWM_FR, pwm_fr);
      putarMotor(IN1_RL, IN2_RL, PWM_RL, pwm_rl);
      putarMotor(IN1_RR, IN2_RR, PWM_RR, pwm_rr);
      
      // // Menyalakan atau mematikan LED
      // digitalWrite(PIN_LED, led_state == 1 ? HIGH : LOW);
    }
  }
}
