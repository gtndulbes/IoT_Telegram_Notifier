// Kode ESP32 untuk Notifikasi Telegram Simulasi via Webhook
// Kode ini membaca suhu dari sensor DHT dan mengirim notifikasi ke webhook simulasi Telegram
// ketika suhu melebihi ambang batas tinggi (mis. 30°C) atau di bawah ambang batas rendah (mis. 10°C). Menggunakan HTTP POST untuk notifikasi.
// Komponen: ESP32, sensor DHT (mis. DHT22), WiFi untuk HTTP POST.
// Library yang diperlukan: WiFi, HTTPClient, DHT (install via Arduino IDE Library Manager).

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Definisi pin
#define DHTPIN 4          // Pin yang terhubung ke pin data sensor DHT
#define DHTTYPE DHT22     // Tipe sensor DHT (ubah ke DHT11 jika menggunakan DHT11)

// Ambang batas untuk kondisi kritis (suhu dalam Celsius)
#define TEMP_HIGH_THRESHOLD 30.0  // Suhu tinggi
#define TEMP_LOW_THRESHOLD 10.0   // Suhu rendah

// Kredensial WiFi (ganti dengan milik Anda)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Detail Webhook Telegram Simulasi (ganti dengan token bot dan chat ID yang sebenarnya untuk Telegram nyata)
// Contoh: Gunakan Telegram Bot API: https://api.telegram.org/bot<TOKEN>/sendMessage
const char* webhookURL = "https://api.telegram.org/botYOUR_BOT_TOKEN/sendMessage";  // Ganti dengan token bot Anda
const char* chatID = "YOUR_CHAT_ID";  // Ganti dengan chat ID Anda

// Objek DHT
DHT dht(DHTPIN, DHTTYPE);

// Variabel
float temperature = 0.0;
bool notificationSent = false;  // Flag untuk menghindari spam notifikasi

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inisialisasi sensor DHT
  dht.begin();
  Serial.println("Sensor DHT diinisialisasi.");

  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung.");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Baca suhu dari sensor DHT
  temperature = dht.readTemperature();
  
  // Periksa apakah pembacaan valid
  if (isnan(temperature)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    delay(2000);
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Periksa kondisi kritis (suhu tinggi atau suhu rendah)
  if ((temperature > TEMP_HIGH_THRESHOLD || temperature < TEMP_LOW_THRESHOLD) && !notificationSent) {
    Serial.println("Kondisi kritis terdeteksi! Mengirim notifikasi...");
    
    // Kirim HTTP POST ke webhook simulasi (Telegram)
    if (sendNotification(temperature)) {
      notificationSent = true;  // Set flag untuk mencegah notifikasi berulang
      Serial.println("Notifikasi berhasil dikirim.");
    } else {
      Serial.println("Gagal mengirim notifikasi. Akan mencoba lagi di siklus berikutnya.");
    }
  } else if (temperature <= TEMP_HIGH_THRESHOLD && temperature >= TEMP_LOW_THRESHOLD) {
    // Reset flag jika suhu kembali normal
    notificationSent = false;
  }

  // Delay sebelum pembacaan berikutnya (mis. 5 detik)
  delay(5000);
}

// Fungsi untuk mengirim notifikasi via HTTP POST
bool sendNotification(float temp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi tidak terhubung. Tidak dapat mengirim notifikasi.");
    return false;
  }

  HTTPClient http;
  http.begin(webhookURL);
  http.addHeader("Content-Type", "application/json");

  // Siapkan payload JSON untuk Telegram Bot API
  String payload = "{";
  payload += "\"chat_id\":\"" + String(chatID) + "\",";
  payload += "\"text\":\"Peringatan: ";
  if (temp > TEMP_HIGH_THRESHOLD) {
    payload += "Suhu tinggi terdeteksi! ";
  } else if (temp < TEMP_LOW_THRESHOLD) {
    payload += "Suhu rendah terdeteksi! ";
  }
  payload += "Suhu saat ini: " + String(temp) + " °C\"";
  payload += "}";

  Serial.print("Mengirim permintaan POST ke: ");
  Serial.println(webhookURL);
  Serial.print("Payload: ");
  Serial.println(payload);

  // Kirim permintaan POST
  int httpResponseCode = http.POST(payload);

  // Periksa respons
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Kode Respons HTTP: ");
    Serial.println(httpResponseCode);
    Serial.print("Respons: ");
    Serial.println(response);
    
    // Telegram API mengembalikan 200 jika sukses
    if (httpResponseCode == 200) {
      http.end();
      return true;
    }
  } else {
    Serial.print("Error dalam permintaan HTTP. Kode: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return false;
}
