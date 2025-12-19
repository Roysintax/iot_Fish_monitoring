# 🐟 IoT Fish Monitoring System

![Project Status](https://img.shields.io/badge/status-active-success.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![IoT](https://img.shields.io/badge/IoT-Blynk-green.svg)

**IoT Fish Monitoring** adalah sistem cerdas berbasis Internet of Things (IoT) untuk memantau dan mengelola kualitas air kolam ikan secara *real-time*. Proyek ini mengintegrasikan sensor kualitas air, aktuator otomatis, dan kamera pemantau untuk memastikan kondisi lingkungan ikan tetap optimal.

---

## 🌟 Fitur Utama

### 1. 📊 Monitoring Kualitas Air
* **Suhu Air:** Memantau suhu menggunakan sensor **DS18B20**.
* **pH Air:** Memantau tingkat keasaman air menggunakan **Sensor pH Analog**.
* Data ditampilkan secara *real-time* di aplikasi **Blynk** dan **LCD 16x2**.

### 2. ⚙️ Otomasi Cerdas
* **Stabilizer pH Otomatis:**
    * Jika **pH < 6**: Servo *pH Up* aktif (menambahkan larutan basa).
    * Jika **pH > 11**: Servo *pH Down* aktif (menambahkan larutan asam).
* **Pemberian Pakan Otomatis (Auto Feeder):**
    * Terjadwal otomatis pada pukul **08:00, 13:00, dan 15:00** (berbasis RTC DS3231 & NTP).
    * Dapat dikontrol manual melalui tombol di aplikasi Blynk.

### 3. 📸 Visual Monitoring & Keamanan
* **Smart Capture:** Sistem otomatis mengambil foto menggunakan **ESP32-CAM** jika terdeteksi kondisi abnormal:
    * Suhu Air terlalu rendah (< 8°C) atau terlalu tinggi (> 30°C).
    * pH Air tidak normal (< 6 atau > 11).
    * Saat pemberian pakan berlangsung.
* **Email Notification:** Foto kondisi kolam dikirim langsung ke email pemilik (`roysihan00@gmail.com`) sebagai laporan bukti.

---

## 🛠️ Perangkat Keras (Hardware)

Sistem ini menggunakan dua mikrokontroler yang bekerja secara paralel:

### 🧠 Master Controller (ESP32)
Bertugas membaca sensor, mengontrol servo, dan mengirim data ke Blynk.
* **Mikrokontroler:** ESP32 Dev Module
* **Sensor:**
    * DS18B20 (Suhu Waterproof)
    * Sensor pH Analog (dengan modul konverter)
* **Aktuator:**
    * Servo Motor (x3): Pakan, pH Up, pH Down
    * LCD 16x2 (I2C)
* **Waktu:** RTC DS3231 (disinkronisasi dengan NTP Server)

### 👁️ Visual Unit (ESP32-CAM)
Bertugas mengambil gambar saat diperintah oleh Master Controller via API.
* **Mikrokontroler:** ESP32-CAM (AI-Thinker)
* **Komunikasi:** HTTP Request ke API Server & SMTP Email Client.

---

## 🔌 Wiring Diagram

Berikut adalah skema rangkaian sistem:

![Wiring Diagram](wiring%20diagram.jpg)

### 📌 Pin Mapping (ESP32 Master)

| Komponen | Pin ESP32 | Keterangan |
| :--- | :---: | :--- |
| **Sensor Suhu (DS18B20)** | GPIO 4 | OneWire Bus |
| **Sensor pH** | GPIO 34 | Analog Input (ADC) |
| **Servo Pakan** | GPIO 12 | PWM Output |
| **Servo pH Up** | GPIO 25 | PWM Output |
| **Servo pH Down** | GPIO 26 | PWM Output |
| **LCD 16x2 & RTC** | GPIO 21 (SDA), 22 (SCL) | I2C Protocol |

---

## 💻 Instalasi & Konfigurasi

### 1. Persiapan Library
Pastikan library berikut terinstall di Arduino IDE Anda:
* `BlynkSimpleEsp32`
* `ESP32Servo`
* `DallasTemperature` & `OneWire`
* `LiquidCrystal_I2C`
* `RTClib`
* `NTPClient`
* `ArduinoJson`
* `ESP32_MailClient` (untuk ESP32-CAM)

### 2. Konfigurasi Kredensial
Edit file `.ino` sesuai dengan kredensial jaringan dan layanan Anda:

**Pada `master.ino`:**
```cpp
#define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
#define BLYNK_DEVICE_NAME "PROJECT KAMPUS"
#define BLYNK_AUTH_TOKEN "xxxxxx"

const char* ssid = "NamaWiFiAnda";
const char* password = "PasswordWiFiAnda";
