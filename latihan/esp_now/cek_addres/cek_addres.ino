#include "WiFi.h"
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("cek");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}
void loop(){}