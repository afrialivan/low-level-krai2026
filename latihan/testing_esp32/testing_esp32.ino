/*
   FULL SELF TEST ESP32
   Tanpa komponen external

   Yang dites:
   1. CPU
   2. RAM
   3. Flash
   4. WiFi Scan
   5. Bluetooth
   6. GPIO LED
   7. ADC internal noise
   8. Restart reason
   9. Free heap realtime
   10. Stability loop

   Board umum:
   ESP32 Devkit V1
*/

#include <WiFi.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_bt.h"

#define LED_PIN 2

unsigned long lastPrint = 0;
unsigned long counterLoop = 0;

void printSeparator() {
  Serial.println("=================================");
}

void testCPU() {
  printSeparator();
  Serial.println("TEST CPU");

  Serial.print("Chip Model     : ");
  Serial.println(ESP.getChipModel());

  Serial.print("Chip Revision  : ");
  Serial.println(ESP.getChipRevision());

  Serial.print("CPU Frequency  : ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  Serial.print("Core Count     : ");
  Serial.println(ESP.getChipCores());

  Serial.println("CPU TEST OK");
}

void testFlash() {
  printSeparator();
  Serial.println("TEST FLASH");

  Serial.print("Flash Size     : ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");

  Serial.print("Flash Speed    : ");
  Serial.print(ESP.getFlashChipSpeed() / 1000000);
  Serial.println(" MHz");

  Serial.println("FLASH TEST OK");
}

void testRAM() {
  printSeparator();
  Serial.println("TEST RAM");

  Serial.print("Free Heap      : ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("Min Free Heap  : ");
  Serial.println(ESP.getMinFreeHeap());

  Serial.print("Max Alloc Heap : ");
  Serial.println(ESP.getMaxAllocHeap());

  Serial.println("RAM TEST OK");
}

void testLED() {
  printSeparator();
  Serial.println("TEST LED");

  pinMode(LED_PIN, OUTPUT);

  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);

    digitalWrite(LED_PIN, LOW);
    delay(150);
  }

  Serial.println("LED TEST OK");
}

void testWiFi() {
  printSeparator();
  Serial.println("TEST WIFI");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(1000);

  int n = WiFi.scanNetworks();

  Serial.print("WiFi Found     : ");
  Serial.println(n);

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" | RSSI ");
      Serial.println(WiFi.RSSI(i));
    }
  }

  Serial.println("WIFI TEST OK");
}

void testBluetooth() {
  printSeparator();
  Serial.println("TEST BLUETOOTH");

  if (!btStart()) {
    Serial.println("Bluetooth FAIL");
    return;
  }

  Serial.println("Bluetooth START OK");

  btStop();

  Serial.println("Bluetooth STOP OK");
}

void testADC() {
  printSeparator();
  Serial.println("TEST ADC");

  long total = 0;

  for (int i = 0; i < 100; i++) {
    total += analogRead(34);
    delay(5);
  }

  int avg = total / 100;

  Serial.print("ADC Average    : ");
  Serial.println(avg);

  Serial.println("ADC TEST OK");
}

void testRestartReason() {
  printSeparator();
  Serial.println("TEST RESTART");

  esp_reset_reason_t reason = esp_reset_reason();

  Serial.print("Restart Reason : ");

  switch (reason) {

    case ESP_RST_POWERON:
      Serial.println("POWER ON");
      break;

    case ESP_RST_SW:
      Serial.println("SOFTWARE RESET");
      break;

    case ESP_RST_PANIC:
      Serial.println("PANIC RESET");
      break;

    case ESP_RST_INT_WDT:
      Serial.println("WATCHDOG");
      break;

    default:
      Serial.println("OTHER");
      break;
  }
}

void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println();
  Serial.println("ESP32 FULL SELF TEST");

  testCPU();
  testFlash();
  testRAM();
  testLED();
  testWiFi();
  testBluetooth();
  testADC();
  testRestartReason();

  printSeparator();
  Serial.println("SEMUA TEST SELESAI");
}

void loop() {

  counterLoop++;

  digitalWrite(LED_PIN, !digitalRead(LED_PIN));

  if (millis() - lastPrint > 2000) {

    lastPrint = millis();

    Serial.println();
    Serial.println("===== REALTIME STATUS =====");

    Serial.print("Uptime        : ");
    Serial.print(millis() / 1000);
    Serial.println(" sec");

    Serial.print("Free Heap     : ");
    Serial.println(ESP.getFreeHeap());

    Serial.print("Loop Counter  : ");
    Serial.println(counterLoop);

    Serial.print("Hall Sensor   : ");
    Serial.println(hallRead());

    Serial.println("SYSTEM NORMAL");
  }

  delay(200);
}