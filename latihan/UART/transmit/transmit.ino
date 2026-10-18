// ESP32 Pengirim UART Serial2

HardwareSerial SerialPort(2);

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);

  SerialPort.begin(115200, SERIAL_8N1, RXD2, TXD2);

  Serial.println("ESP32 Pengirim Siap");
}

void loop() {
  String data = "Halo dari ESP32 Pengirim";

  // SerialPort.println(data);

  SerialPort.println("200,200,200,200");
  Serial.println(data);

  delay(2000);
}