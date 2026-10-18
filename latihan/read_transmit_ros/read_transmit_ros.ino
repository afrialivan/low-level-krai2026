int led1 = 13;
int led2 = 12;

String data = "";

// 🔹 variabel bebas
int led1_state = 0;
int led2_state = 0;

// 🔹 buffer parsing
int values[10];  // maksimal 10 data

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  Serial.begin(9600);
}

void loop() {

  // ==============================
  // 🔹 TERIMA DATA DARI ROS
  // ==============================
  if (Serial.available() > 0) {

    data = Serial.readStringUntil('\n');

    int index = 0;
    int start = 0;
    int end = data.indexOf(',');

    // parsing semua data
    while (end != -1) {
      if (index < 10) {
        values[index++] = data.substring(start, end).toInt();
      }

      start = end + 1;
      end = data.indexOf(',', start);
    }

    // ambil data terakhir
    if (index < 10) {
      values[index++] = data.substring(start).toInt();
    }

    // ==============================
    // 🔹 SIMPAN KE VARIABEL
    // ==============================
    if (index >= 2) {
      led1_state = values[0];
      led2_state = values[1];
    }

    // contoh kalau nanti nambah:
    // if (index >= 3) motor_speed = values[2];
    // if (index >= 4) servo_angle = values[3];
  }

  // ==============================
  // 🔹 EKSEKUSI OUTPUT
  // ==============================
  digitalWrite(led1, led1_state);
  digitalWrite(led2, led2_state);

  // ==============================
  // 🔹 KIRIM DATA KE ROS
  // ==============================
  float suhu = 1.5 + (random(1000) / 1000.0) * (5.5 - 1.5);
  float kelembaban = 1.5 + (random(1000) / 1000.0) * (5.5 - 1.5);
  int jarak = random(1000);

  Serial.print("S:");
  Serial.print(suhu);
  Serial.print(",H:");
  Serial.print(kelembaban);
  Serial.print(",J:");
  Serial.println(jarak);

  delay(200);
}
