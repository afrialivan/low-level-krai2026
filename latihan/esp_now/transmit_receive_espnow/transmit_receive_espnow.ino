#include <esp_now.h>
#include <WiFi.h>

// GANTI DENGAN MAC ADDRESS LAWAN (Board tujuan)
uint8_t broadcastAddress[] = {0x28, 0x05, 0xA5, 0x6F, 0x53, 0xE0};

// Struktur pesan untuk dikirim dan diterima
typedef struct struct_message {
    char msg[32];
    int reading;
} struct_message;

struct_message myData;      // Data yang akan dikirim
struct_message incomingData; // Data yang diterima

esp_now_peer_info_t peerInfo;

// Callback saat data terkirim
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus Pengiriman Terakhir: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sukses Terkirim" : "Gagal Terkirim");
}

// Callback saat data diterima
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingRawData, int len) {
  memcpy(&incomingData, incomingRawData, sizeof(incomingData));
  Serial.print("Pesan Diterima: ");
  Serial.println(incomingData.msg);
  Serial.print("Nilai Angka: ");
  Serial.println(incomingData.reading);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Gagal menginisialisasi ESP-NOW");
    return;
  }

  // Daftarkan fungsi callback
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Daftarkan Peer (Penerima)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Gagal menambahkan peer");
    return;
  }
}

void loop() {
  // Isi data yang ingin dikirim
  strcpy(myData.msg, "Halo dari ESP32!");
  myData.reading = random(0, 100);

  // Kirim pesan via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Pesan terkirim dengan aman");
  } else {
    Serial.println("Error saat mengirim data");
  }

  delay(2000); // Kirim setiap 2 detik
}