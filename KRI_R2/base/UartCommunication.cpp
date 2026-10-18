#include "UartCommunication.h"
#include "MotorControl.h"
#include "Encoder.h"

// Menggunakan Serial2 (Hardware Serial ESP32)
// Default pin RX = 16, TX = 17
#define UART_BAUDRATE 115200

void setupUART() {
  Serial2.begin(UART_BAUDRATE);
}

void processUART() {
  // Cek apakah ada data masuk dari Master via UART
  if (Serial2.available() > 0) {
    String data = Serial2.readStringUntil('\n');
    
    int parsedFL, parsedFR, parsedRL, parsedRR;
    
    // Parse data berformat: FL,FR,RL,RR
    if (sscanf(data.c_str(), "%d,%d,%d,%d", &parsedFL, &parsedFR, &parsedRL, &parsedRR) == 4) {
      motFL.setpoint = parsedFL;
      motFR.setpoint = parsedFR;
      motRL.setpoint = parsedRL;
      motRR.setpoint = parsedRR;
    }
  }
}

void sendFeedbackUART() {
  // Kirim feedback ke Master dengan format:
  // RPM_FL,RPM_FR,RPM_RL,RPM_RR,COUNT_FL,COUNT_FR,COUNT_RL,COUNT_RR\n
  Serial2.printf("%.2f,%.2f,%.2f,%.2f,%ld,%ld,%ld,%ld\n",
                 motFL.rpm, motFR.rpm, motRL.rpm, motRR.rpm,
                 encFL.count, encFR.count, encRL.count, encRR.count);
}