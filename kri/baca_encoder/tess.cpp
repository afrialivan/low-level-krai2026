// // m2
// #define ENC1_A 25
// #define ENC1_B 15
// //m4
// #define ENC2_A 39
// #define ENC2_B 36

// #define PPR 193
// long lastCount1 = 0;
// long lastCount2 = 0;
// unsigned long lastTime = 0;

// volatile long encCount1 = 0;
// volatile long encCount2 = 0;
// volatile int lastEnc1 = 0;
// volatile int lastEnc2 = 0;


// void IRAM_ATTR encoder1ISR() {
//   int A = digitalRead(ENC1_A);
//   int B = digitalRead(ENC1_B);
//   int encoded = (A << 1) | B;
//   int sum = (lastEnc1 << 2) | encoded;

//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
//     encCount1++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
//     encCount1--;

//   lastEnc1 = encoded;
// }

// void IRAM_ATTR encoder2ISR() {
//   int A = digitalRead(ENC2_A);
//   int B = digitalRead(ENC2_B);
//   int encoded = (A << 1) | B;
//   int sum = (lastEnc2 << 2) | encoded;

//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
//     encCount2++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
//     encCount2--;

//   lastEnc2 = encoded;
// }


// void setup() {
//   Serial.begin(115200);

//   pinMode(ENC1_A, INPUT);
//   pinMode(ENC1_B, INPUT);
//   pinMode(ENC2_A, INPUT);
//   pinMode(ENC2_B, INPUT);

//   attachInterrupt(digitalPinToInterrupt(ENC1_A), encoder1ISR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(ENC1_B), encoder1ISR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(ENC2_A), encoder2ISR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(ENC2_B), encoder2ISR, CHANGE);

//   lastTime = millis();
// }

// void loop() {
//   unsigned long now = millis();

//   if (now - lastTime >= 100) {
//     //long delta = encoderCount - lastCount;
//     //float rpm = (delta * 600.0) / PPR;

//     long d1 = encCount1 - lastCount1;
//     long d2 = encCount2 - lastCount2;
//     lastCount1 = encCount1;
//     lastCount2 = encCount2;

//     float rpm1 = (d1 * 600.0) / PPR;
//     float rpm2 = -(d2 * 600.0) / PPR;

//     Serial.print("Count1: ");
//     Serial.print(encCount1);
//     Serial.print(" | RPM:1 ");
//     Serial.print(rpm1);
//     Serial.print("    ||    ");
//     Serial.print("Count2: ");
//     Serial.print(encCount2);
//     Serial.print(" | RPM2: ");
//     Serial.println(rpm2);

//     lastCount1 = encCount1;
//     lastCount2 = encCount2;
//     lastTime = now;
//   }
// }
