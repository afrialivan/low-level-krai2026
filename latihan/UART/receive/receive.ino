// ESP32 Penerima UART Serial2

HardwareSerial SerialPort(2);

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);

  SerialPort.begin(115200, SERIAL_8N1, RXD2, TXD2);

  Serial.println("ESP32 Penerima Siap");
}

void loop() {
  while (SerialPort.available()) {
    String dataMasuk = SerialPort.readStringUntil('\n');

    Serial.println("Data diterima:");
    Serial.println(dataMasuk);
  }
}