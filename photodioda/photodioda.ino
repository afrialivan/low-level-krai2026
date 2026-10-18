const int photodiodePin = 34; 
int lightValue = 0;

void setup() {
  // Inisialisasi komunikasi serial dengan baudrate 115200
  Serial.begin(115200);
  
  analogReadResolution(12);
  
  Serial.println("Memulai pembacaan sensor photodioda...");
}

void loop() {
  // Membaca nilai tegangan analog dari Vout
  lightValue = analogRead(photodiodePin);
  
  // Mencetak nilai ke Serial Monitor
  Serial.print("Intensitas Cahaya (Raw ADC): ");
  Serial.println(lightValue);
  
  // Nilai akan mendekati 0 saat gelap, dan mendekati 4095 saat sangat terang
  
  delay(500); // Jeda pembacaan selama 500 milidetik
}