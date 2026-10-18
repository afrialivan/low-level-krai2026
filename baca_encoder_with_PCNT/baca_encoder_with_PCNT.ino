#include "driver/pcnt.h"

#define ENC1_A 32
#define ENC1_B 33
#define ENC2_A 34
#define ENC2_B 35

#define PPR 193

#define M1_PWM 13
#define M1_IN1 14
#define M1_IN2 12

#define M2_PWM 25
#define M2_IN1 27
#define M2_IN2 26

long lastCount1 = 0;
long lastCount2 = 0;
unsigned long lastTime = 0;

void setupPCNT() {
  pcnt_config_t pcnt_config1 = {};
  pcnt_config1.pulse_gpio_num = ENC1_A;
  pcnt_config1.ctrl_gpio_num = ENC1_B;
  pcnt_config1.channel = PCNT_CHANNEL_0;
  pcnt_config1.unit = PCNT_UNIT_0;
  pcnt_config1.pos_mode = PCNT_COUNT_INC;
  pcnt_config1.neg_mode = PCNT_COUNT_DIS;
  pcnt_config1.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config1.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config1.counter_h_lim = 32767;
  pcnt_config1.counter_l_lim = -32768;

  pcnt_unit_config(&pcnt_config1);

  pcnt_set_filter_value(PCNT_UNIT_0, 100);
  pcnt_filter_enable(PCNT_UNIT_0);

  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0);


  pcnt_config_t pcnt_config2 = {};
  pcnt_config2.pulse_gpio_num = ENC2_A;
  pcnt_config2.ctrl_gpio_num = ENC2_B;
  pcnt_config2.channel = PCNT_CHANNEL_0;
  pcnt_config2.unit = PCNT_UNIT_1;
  pcnt_config2.pos_mode = PCNT_COUNT_INC;
  pcnt_config2.neg_mode = PCNT_COUNT_DIS;
  pcnt_config2.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config2.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config2.counter_h_lim = 32767;
  pcnt_config2.counter_l_lim = -32768;

  pcnt_unit_config(&pcnt_config2);

  pcnt_set_filter_value(PCNT_UNIT_1, 100);
  pcnt_filter_enable(PCNT_UNIT_1);

  pcnt_counter_pause(PCNT_UNIT_1);
  pcnt_counter_clear(PCNT_UNIT_1);
  pcnt_counter_resume(PCNT_UNIT_1);
}

void setup() {
  Serial.begin(115200);

  pinMode(ENC1_A, INPUT);
  pinMode(ENC1_B, INPUT);
  pinMode(ENC2_A, INPUT);
  pinMode(ENC2_B, INPUT);

  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);

  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);

  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);

  analogWrite(M1_PWM, 100);
  analogWrite(M2_PWM, 100);

  setupPCNT();

  lastTime = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastTime >= 100) {
    int16_t count1 = 0;
    int16_t count2 = 0;

    pcnt_get_counter_value(PCNT_UNIT_0, &count1);
    pcnt_get_counter_value(PCNT_UNIT_1, &count2);

    long d1 = count1 - lastCount1;
    long d2 = count2 - lastCount2;

    lastCount1 = count1;
    lastCount2 = count2;

    float rpm1 = (d1 * 600.0) / PPR;
    float rpm2 = -(d2 * 600.0) / PPR;

    Serial.print("Count1: ");
    Serial.print(count1);
    Serial.print(" | RPM1: ");
    Serial.print(rpm1);
    Serial.print("    ||    ");
    Serial.print("Count2: ");
    Serial.print(count2);
    Serial.print(" | RPM2: ");
    Serial.println(rpm2);

    lastTime = now;
  }
}
