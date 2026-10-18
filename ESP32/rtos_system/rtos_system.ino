#include <Arduino.h>

// --- 1. DEKLARASI 3 QUEUE ---
QueueHandle_t qSortir;   // Antrean barang masuk ke bagian sortir
QueueHandle_t qKemas;    // Antrean barang dari sortir ke bagian pengemasan
QueueHandle_t qKirim;    // Antrean barang dari kemas ke bagian pengiriman

// --- 2. DEKLARASI 3 SEMAPHORE ---
SemaphoreHandle_t semIzinSortir;
SemaphoreHandle_t semIzinKemas;
SemaphoreHandle_t semIzinKirim;

// Prototipe Task
void TaskBagianSortir(void *pvParameters);
void TaskBagianKemas(void *pvParameters);
void TaskBagianKirim(void *pvParameters);
void TaskSimulasiBarangMasuk(void *pvParameters);

void setup() {
  Serial.begin(115200);
  Serial.println("=== SISTEM GUDANG OTOMATIS DIMULAI ===");

  // --- 3. INISIALISASI 3 QUEUE ---
  qSortir = xQueueCreate(5, sizeof(int)); // Menampung hingga 5 ID Barang
  qKemas  = xQueueCreate(5, sizeof(int));
  qKirim  = xQueueCreate(5, sizeof(int));

  // --- 4. INISIALISASI 3 SEMAPHORE ---
  semIzinSortir = xSemaphoreCreateBinary();
  semIzinKemas  = xSemaphoreCreateBinary();
  semIzinKirim  = xSemaphoreCreateBinary();

  // Aktifkan semua mesin di awal (Beri tiket pertama)
  xSemaphoreGive(semIzinSortir);
  xSemaphoreGive(semIzinKemas);
  xSemaphoreGive(semIzinKirim);

  // --- 5. REGISTER 4 TASK ---
  xTaskCreate(TaskSimulasiBarangMasuk, "Input_Barang", 2048, NULL, 1, NULL);
  xTaskCreate(TaskBagianSortir,        "Mesin_Sortir", 2048, NULL, 2, NULL);
  xTaskCreate(TaskBagianKemas,         "Mesin_Kemas",  2048, NULL, 2, NULL);
  xTaskCreate(TaskBagianKirim,         "Mesin_Kirim",  2048, NULL, 2, NULL);
}

void loop() {
  // Kosong, sepenuhnya dikendalikan RTOS
}

/* ==================== DEFINISI TASKS ==================== */

// TASK PEMICU: Mensimulasikan ada barang baru masuk gudang setiap 5 detik
void TaskSimulasiBarangMasuk(void *pvParameters) {
  int idBarang = 100; // ID awal barang
  for(;;) {
    idBarang++; // ID Barang jadi 101, 102, dst.
    Serial.print("\n[DOCK GUDANG] -> Barang Baru Datang! ID: ");
    Serial.println(idBarang);

    // Kirim ID barang ke antrean sortir
    xQueueSend(qSortir, &idBarang, 0);

    vTaskDelay(pdMS_TO_TICKS(5000)); // Barang baru datang setiap 5 detik
  }
}

// TASK 1: Bagian Sortir
void TaskBagianSortir(void *pvParameters) {
  int barangDiterima;
  for(;;) {
    // Syarat 1: Harus ada barang di qSortir
    if (xQueueReceive(qSortir, &barangDiterima, portMAX_DELAY)) {
      // Syarat 2: Harus dapat izin/tiket dari semIzinSortir
      if (xSemaphoreTake(semIzinSortir, portMAX_DELAY)) {
        
        Serial.print("[1. SORTIR]  -> Menyortir barang ID: ");
        Serial.println(barangDiterima);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Proses sortir butuh 1 detik
        
        // Selesai sortir, kirim ke antrean pengemasan (qKemas)
        xQueueSend(qKemas, &barangDiterima, 0);
        
        // Kembalikan tiket izin kerja agar bisa dipakai barang berikutnya
        xSemaphoreGive(semIzinSortir);
      }
    }
  }
}

// TASK 2: Bagian Pengemasan
void TaskBagianKemas(void *pvParameters) {
  int barangDiterima;
  for(;;) {
    // Syarat 1: Harus ada barang di qKemas
    if (xQueueReceive(qKemas, &barangDiterima, portMAX_DELAY)) {
      // Syarat 2: Harus dapat izin/tiket dari semIzinKemas
      if (xSemaphoreTake(semIzinKemas, portMAX_DELAY)) {
        
        Serial.print("[2. KEMAS]   -> Membungkus barang ID: ");
        Serial.println(barangDiterima);
        vTaskDelay(pdMS_TO_TICKS(1500)); // Proses bungkus butuh 1.5 detik
        
        // Selesai dibungkus, kirim ke antrean pengiriman (qKirim)
        xQueueSend(qKirim, &barangDiterima, 0);
        
        // Kembalikan tiket izin kerja
        xSemaphoreGive(semIzinKemas);
      }
    }
  }
}

// TASK 3: Bagian Pengiriman
void TaskBagianKirim(void *pvParameters) {
  int barangDiterima;
  for(;;) {
    // Syarat 1: Harus ada barang di qKirim
    if (xQueueReceive(qKirim, &barangDiterima, portMAX_DELAY)) {
      // Syarat 2: Harus dapat izin/tiket dari semIzinKirim
      if (xSemaphoreTake(semIzinKirim, portMAX_DELAY)) {
        
        Serial.print("[3. KIRIM]   -> Menempel resi & Kirim barang ID: ");
        Serial.println(barangDiterima);
        vTaskDelay(pdMS_TO_TICKS(800)); // Proses tempel resi butuh 0.8 detik
        
        Serial.print("[SUKSES]     -> Barang ID: ");
        Serial.print(barangDiterima);
        Serial.println(" KELUAR GUDANG (TERKIRIM)!\n");
        
        // Kembalikan tiket izin kerja
        xSemaphoreGive(semIzinKirim);
      }
    }
  }
}