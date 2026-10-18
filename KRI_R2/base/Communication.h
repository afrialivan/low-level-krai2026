#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

extern uint8_t masterAddress[];

typedef struct struct_message {
  int fl; int fr; int rl; int rr;
} struct_message;

typedef struct struct_feedback {
  float rpmFL; float rpmFR; float rpmRL; float rpmRR;
  long countFL; long countFR; long countRL; long countRR;
} struct_feedback;

extern struct_message incomingData;
extern struct_feedback feedbackToMaster;

void setupESPNow();
void sendFeedback();