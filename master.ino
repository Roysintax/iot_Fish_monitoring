// Kodingan Esp 32 
// Mendefinisikan ID Template Blynk   
#define BLYNK_TEMPLATE_ID "TMPL6VJ9bOtPI" // ID Template untuk 
proyek Blynk 
// Mendefinisikan Nama Template Blynk   
#define BLYNK_TEMPLATE_NAME "PROJECT KAMPUS"  // Nama Template untuk 
proyek Blynk 
// Mendefinisikan Blynk Auth Token   
#define BLYNK_AUTH_TOKEN "pNe6RBLtCjUJQz2r9urTwReObi9CNjJT"  // 
Token otentikasi untuk Blynk 
 
#include <OneWire.h>              // Mengimpor library OneWire untuk 
sensor DS18B20 
#include <DallasTemperature.h>    // Mengimpor library 
DallasTemperature untuk membaca sensor DS18B20 
#include <ESP32Servo.h>           // Mengimpor library ESP32Servo 
untuk kontrol servo 
#include <WiFi.h>                 // Mengimpor library WiFi untuk 
koneksi nirkabel 
#include <HTTPClient.h>           // Mengimpor library HTTPClient 
untuk melakukan request HTTP 
#include <ArduinoJson.h>          // Mengimpor library ArduinoJson 
untuk parsing JSON 
#include <RTClib.h>               // Mengimpor library RTClib untuk 
RTC DS3231 
#include <Wire.h>                 // Mengimpor library Wire untuk 
komunikasi I2C 
#include <LiquidCrystal_I2C.h>    // Mengimpor library 
LiquidCrystal_I2C untuk LCD via I2C 
#include <BlynkSimpleEsp32.h>     // Mengimpor library 
BlynkSimpleEsp32 untuk integrasi dengan Blynk 
 
// Tambahan untuk NTP Client 
#include <NTPClient.h>            // Mengimpor library NTPClient 
untuk sinkronisasi waktu via NTP 
#include <WiFiUdp.h>              // Mengimpor library WiFiUdp untuk 
komunikasi NTP 
 
// Mendefinisikan pin ADC untuk sensor pH   
#define pH_PIN 34                 // Sensor pH terhubung ke ADC 34 
 
#define ONE_WIRE_BUS 4            // Sensor suhu DS18B20 menggunakan 
OneWire di GPIO 4 

// Membuat objek OneWire dan sensor DS18B20 
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire); 
 
// Servo untuk pemberian pakan (feed) pada GPIO 12 
int feedServoPin = 12;             
Servo feedServo;                   
 
// Servo untuk pengaturan pH 
int servoPHUpPin = 25;            // Servo untuk menaikkan pH (pH 
Up) pada GPIO 25 
int servoPHDownPin = 26;          // Servo untuk menurunkan pH (pH 
Down) pada GPIO 26 
Servo servoPHUp;                  // Objek servo untuk pH Up 
Servo servoPHDown;                // Objek servo untuk pH Down 
 
// Inisialisasi LCD I2C (alamat 0x27, ukuran 16x2) 
LiquidCrystal_I2C lcd(0x27, 16, 2); 
// Objek RTC DS3231 
RTC_DS3231 rtc; 
 
// Kredensial WiFi dan URL API 
const char* ssid = "Home";      // SSID WiFi //"🍎99" //"Home" 
//Infinix HOT 30 ; 
const char* password = "Home1122**";         // Password WiFi 
//"1234567***" //"Home1122**" //esp32cam;     
const char* apiUrl = "https://databasesih.com/api/camera";       // 
URL API untuk perintah gambar 
const char* apiUrlUpdate = "https://databasesih.com/api/camera/1"; 
// URL API untuk update status capture 
 
// Konstanta ADC: Tegangan referensi dan resolusi ADC ESP32 
const float VREF = 3.3;           
const int ADC_RES = 4095;         
 
// Kalibrasi sensor pH: nilai tegangan pada titik kalibrasi 
const float pH4_VOLTAGE = 3.30;    
const float pH7_VOLTAGE = 2.69;    
const float pH9_VOLTAGE = 2.23;    
 
// Variabel dan flag status pemberian pakan 
bool tempLowPhotoTaken = false;   // Flag agar gambar kondisi suhu 
rendah hanya diambil sekali 
bool tempHighPhotoTaken = false;  // Flag agar gambar kondisi suhu 
tinggi hanya diambil sekali 
bool feedPhotoTaken = false;      // Flag agar gambar pemberian 
pakan hanya diambil sekali 

bool feedActive = false;          // Flag untuk menandakan mekanisme 
pemberian pakan otomatis aktif 
String lastFeedTimestamp = "";    // Menyimpan timestamp terakhir 
pemberian pakan 
bool feedTriggered = false;       // Flag untuk mencegah trigger 
berulang dalam 1 detik 
 
// NTP Client setup: Menggunakan WiFiUDP dan NTPClient dengan offset 
UTC+7 dan update tiap 60 detik 
WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); 
 
// Fungsi untuk menyambungkan ke WiFi 
void connectToWiFi() { 
  Serial.print("Menghubungkan ke WiFi"); 
  WiFi.begin(ssid, password);                   // Mulai koneksi ke 
WiFi dengan SSID dan password 
  while (WiFi.status() != WL_CONNECTED) {         // Tunggu hingga 
WiFi terhubung 
    delay(1000); 
    Serial.print("."); 
  } 
  Serial.println("\nWiFi terhubung."); 
  Serial.print("IP Address: "); 
  Serial.println(WiFi.localIP()); 
} 
 
// Fungsi untuk menyinkronkan RTC dengan NTP 
void syncRTCWithNTP() { 
  while (!timeClient.update()) {                // Tunggu hingga NTP 
client mendapatkan waktu terbaru 
    timeClient.forceUpdate(); 
  } 
  unsigned long epochTime = timeClient.getEpochTime();  // Dapatkan 
waktu epoch dari NTP 
  DateTime ntpTime = DateTime(epochTime);         // Buat objek 
DateTime dari epoch 
  rtc.adjust(ntpTime);                            // Atur waktu RTC 
dengan waktu dari NTP 
  Serial.print("RTC disinkronkan dengan NTP: "); 
  Serial.println(ntpTime.timestamp()); 
} 
 
// Fungsi membaca sensor pH: Mengambil 10 sampel, menghitung rata
rata, lalu mengkonversinya ke Volt 
float readSensor(int pin) { 

      int sampleCount = 10;           // Jumlah sampel untuk mengurangi 
noise 
  float sum = 0;                  // Total pembacaan ADC 
  for (int i = 0; i < sampleCount; i++) { 
    sum += analogRead(pin);       // Baca nilai ADC dari sensor dan 
tambahkan ke sum 
    delay(10);                    // Delay antar pembacaan 
  } 
  float avgADC = sum / sampleCount;  // Hitung rata-rata pembacaan 
ADC 
  return avgADC * (VREF / ADC_RES);  // Konversi nilai rata-rata ke 
Volt 
} 
 
// Fungsi menghitung nilai pH dengan interpolasi linear 
float calculatePH(float voltage) { 
  float phValue;  // Variabel untuk menyimpan nilai pH yang dihitung 
  if (voltage >= pH7_VOLTAGE) {   
    // Interpolasi untuk nilai di atas pH 7 
    phValue = 6.86 + (voltage - pH7_VOLTAGE) * (9.18 - 6.86) / 
(pH9_VOLTAGE - pH7_VOLTAGE); 
  } else { 
    // Interpolasi untuk nilai di bawah pH 7 
    phValue = 4.01 + (voltage - pH4_VOLTAGE) * (6.86 - 4.01) / 
(pH7_VOLTAGE - pH4_VOLTAGE); 
  } 
  return phValue;  // Kembalikan nilai pH yang dihitung 
} 
 
// Fungsi pengaturan servo untuk pengendalian pH menggunakan dua 
servo 
// Jika pH < 5, servoPHUp bergerak; jika pH > 11, servoPHDown 
bergerak; jika pH normal (6-9), kedua servo kembali ke 0° 
void adjustPH(float pH) { 
  if (pH < 6) {  // pH rendah 
    Serial.println("pH rendah: Menggerakkan servo pH Up..."); 
    servoPHUp.write(120);  // Gerakkan servo untuk menaikkan pH 
    Blynk.virtualWrite(V5, 120); // Update virtual pin V5 ke 90 
(aktif) 
    delay(7000);          // Delay agar aksi terlihat 
    servoPHUp.write(0);   // Kembalikan servo ke posisi 0 
     Blynk.virtualWrite(V5, 0);// Update virtual pin V5 ke 0 (non
aktif) 
  } else if (pH > 11) {  // pH tinggi 
    Serial.println("pH tinggi: Menggerakkan servo pH Down..."); 
    servoPHDown.write(120);  // Gerakkan servo untuk menurunkan pH

 Blynk.virtualWrite(V6, 120); // Update virtual pin V6 ke 90 
(aktif) 
    delay(7000);            // Delay agar aksi terlihat 
    servoPHDown.write(0);   // Kembalikan servo ke posisi 0 
    Blynk.virtualWrite(V6, 0);  // Update virtual pin V6 ke 0 (non
aktif) 
  } else {  // pH normal 
    servoPHUp.write(0); 
    servoPHDown.write(0); 
  } 
} 
 
// Fungsi evaluasi kondisi untuk pengambilan gambar (sensor 
turbidity tidak aktif) 
void perintahGambar(float pH, float tempC) { 
  bool takePic = false;  // Flag untuk menentukan apakah gambar 
perlu diambil 
   
  if (pH < 6 || pH > 11) {  // Jika nilai pH tidak normal 
    takePic = true; 
  } 
  if (tempC < 8) {  // Jika suhu rendah 
    if (!tempLowPhotoTaken) { 
      takePic = true; 
      tempLowPhotoTaken = true; 
    } 
  } else { 
    tempLowPhotoTaken = false;  // Reset flag suhu rendah 
  } 
  if (tempC > 30) {  // Jika suhu tinggi 
    if (!tempHighPhotoTaken) { 
      takePic = true; 
      tempHighPhotoTaken = true; 
    } 
  } else { 
    tempHighPhotoTaken = false;  // Reset flag suhu tinggi 
  } 
  if (feedActive) {  // Jika mekanisme pemberian pakan aktif 
    if (!feedPhotoTaken) { 
      takePic = true; 
      feedPhotoTaken = true; 
    } 
  } else { 
    feedPhotoTaken = false;  // Reset flag pemberian pakan 
  } 
   
  if (takePic) {

     Serial.println("Perintah gambar dijalankan..."); 
    getData(pH);  // Panggil fungsi untuk memproses pengambilan 
gambar 
    delay(500);   // Delay singkat setelah pengambilan gambar 
  } 
} 
 
// Fungsi untuk mengambil data perintah gambar dari API 
void getData(float pH) { 
  if (WiFi.status() == WL_CONNECTED) {  // Pastikan WiFi terhubung 
    HTTPClient http; 
    http.begin(apiUrl);  // Mulai koneksi ke URL API 
    int httpResponseCode = http.GET();  // Lakukan HTTP GET 
    if (httpResponseCode > 0) { 
      String response = http.getString();  // Ambil respons dari API 
      DynamicJsonDocument doc(1024); 
      deserializeJson(doc, response);  // Parsing respons JSON 
      int captureValue = doc["data"]["capture"];  // Ambil nilai 
capture dari JSON 
      Serial.print("Nilai Capture: "); 
      Serial.println(captureValue); 
      if (captureValue == 0) {  // Jika capture = 0, ambil gambar 
        takePicture(); 
      } else { 
        Serial.println("Sudah diperintahkan untuk ambil gambar!"); 
      } 
    } else { 
      Serial.print("Gagal mengambil data. Kode error: "); 
      Serial.println(httpResponseCode); 
    } 
    http.end();  // Tutup koneksi HTTP 
  } 
} 
 
// Fungsi untuk mengirim perintah ke API agar kamera mengambil 
gambar 
void takePicture(){ 
  if (WiFi.status() == WL_CONNECTED) { 
    HTTPClient http; 
    http.begin(apiUrlUpdate);  // Mulai koneksi ke URL API update 
    int httpResponseCode = http.GET();  // Lakukan HTTP GET 
    if (httpResponseCode > 0) { 
      Serial.print("Berhasil Update!");  // Gambar berhasil diambil 
    } else { 
      Serial.print("Gagal mengambil data. Kode error: "); 
      Serial.println(httpResponseCode); 
    } 
 http.end();  // Tutup koneksi HTTP 
  } 
} 
 
// Fungsi untuk membaca nilai suhu dari sensor DS18B20 dan mengirim 
nilainya ke Blynk 
void suhu() { 
  sensors.requestTemperatures();  // Minta pembacaan suhu terbaru 
dari sensor 
  float tempC = sensors.getTempCByIndex(0);  // Ambil suhu (°C) dari 
sensor pertama 
  Serial.print("Suhu: "); 
  Serial.print(tempC); 
  Serial.println(" °C"); 
  Blynk.virtualWrite(V2, tempC);  // Kirim nilai suhu ke virtual pin 
V2 di Blynk 
} 
 
// Fungsi untuk membuat laporan notifikasi gabungan yang mencakup 
nilai sensor dan status sistem 
void logCombinedReport(float pH, float tempC) { 
  String phStr; 
  if (pH < 6.68) { 
    phStr = "pH: rendah (" + String(pH, 2) + ")"; 
  } else if (pH > 11) { 
    phStr = "pH: tinggi (" + String(pH, 2) + ")"; 
  } else { 
    phStr = "pH: Normal (" + String(pH, 2) + ")"; 
  } 
   
  String tempStr; 
  if (tempC < 8) { 
    tempStr = "Suhu: rendah (" + String(tempC, 1) + "°C)"; 
  } else if (tempC > 30) { 
    tempStr = "Suhu: tinggi (" + String(tempC, 1) + "°C)"; 
  } else { 
    tempStr = "Suhu: Normal (" + String(tempC, 1) + "°C)"; 
  } 
   
  // Karena sensor kekeruhan tidak aktif, tidak ada informasi 
kekeruhan 
  String feedStr; 
  if (lastFeedTimestamp != "") { 
    feedStr = "Pakan: " + lastFeedTimestamp + " - Sudah diberi 
pakan"; 
  } else { 
    feedStr = "Pakan: Tidak ada pemberian pakan"; 
 } 
   
  bool allNormal = ((pH >= 6.68 && pH <= 10) && (tempC >= 10 && 
tempC <= 40)); 
  String kondisi = allNormal ? "Kondisi Normal" : "Kondisi 
Abnormal"; 
   
  String report = kondisi + "\n" + 
                  feedStr + "\n" + 
                  phStr + "\n" + 
                  tempStr; 
   
  Serial.println("Mengirim notifikasi ke Blynk..."); 
  Serial.println(report); 
   
  Blynk.logEvent("project_kampus", report);  // Kirim log event ke 
Blynk 
} 
 
// Callback Virtual Pin V7: Button untuk pemberian pakan manual 
BLYNK_WRITE(V7) { 
  int buttonState = param.asInt();  // Baca nilai button dari 
aplikasi Blynk 
  Serial.print("Nilai button V7: "); 
  Serial.println(buttonState); 
  if (buttonState == 1) {  // Jika button ditekan 
    Serial.println("Button V7 ditekan: Membuka servo feed (pemberian 
pakan)..."); 
    feedServo.write(40);  // Servo feed bergerak ke posisi 90° (buka 
pakan) 
    delay(1000);          // Delay agar aksi terlihat 
    feedServo.write(0);   // Kembalikan servo feed ke posisi 0° 
(tutup pakan) 
    // Update timestamp pemberian pakan 
    DateTime now = rtc.now(); 
    char datetime[20]; 
    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", 
            now.year(), now.month(), now.day(), now.hour(), 
now.minute(), now.second()); 
    lastFeedTimestamp = String(datetime); 
  } 
} 
 
void setup() { 
  Serial.begin(115200);                           // Mulai 
komunikasi serial dengan baud rate 115200 
 Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);  // Inisialisasi 
koneksi ke Blynk 
   
  if (WiFi.status() == WL_CONNECTED) {            // Tampilkan 
status WiFi 
    Serial.println("WiFi terhubung"); 
  } else { 
    Serial.println("WiFi tidak terhubung"); 
  } 
   
  // Attach servo feed ke GPIO 12 
  feedServo.attach(feedServoPin); 
  // Attach servo pH ke GPIO 25 dan 26 
  servoPHUp.attach(servoPHUpPin); 
  servoPHDown.attach(servoPHDownPin); 
   
  sensors.begin();  // Inisialisasi sensor DS18B20 
   
  // Inisialisasi RTC DS3231 via I2C pada pin 21 dan 22 
  Wire.begin(21, 22); 
  if (!rtc.begin()) { 
    Serial.println("RTC tidak terdeteksi!"); 
    while (1); 
  } 
   
  // Sambungkan ke WiFi untuk NTP dan sinkronisasi RTC 
  connectToWiFi(); 
  timeClient.begin(); 
  syncRTCWithNTP(); 
   
  // Inisialisasi pin output untuk relay pH telah dihapus 
   
  lcd.init();         // Inisialisasi LCD I2C 
  lcd.backlight();    // Nyalakan backlight LCD 
} 
 
void loop() { 
  Blynk.run();  // Jalankan loop Blynk untuk komunikasi 
   
  // Perbarui waktu RTC setiap 1 jam 
  static unsigned long lastSync = 0; 
  if (millis() - lastSync > 3600000) {  // Jika lebih dari 1 jam 
    Serial.println("Memperbarui waktu RTC dengan NTP..."); 
    syncRTCWithNTP(); 
    lastSync = millis(); 
  } 
   DateTime currentTime = rtc.now();  // Ambil waktu saat ini dari 
RTC 
   
  // Evaluasi kondisi pemberian pakan otomatis berdasarkan waktu 
  if ( 
    ((currentTime.hour() == 8) && (currentTime.minute() == 0) && 
(currentTime.second() == 0)) || 
    ((currentTime.hour() == 13) && (currentTime.minute() == 0) && 
(currentTime.second() == 0)) || 
    ((currentTime.hour() == 15) && (currentTime.minute() == 0) && 
(currentTime.second() == 0)) 
  ) { 
    if (!feedTriggered) { 
      feedActive = true;  // Aktifkan mekanisme pemberian pakan 
otomatis 
      char datetime[20]; 
      sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", 
              currentTime.year(), currentTime.month(), 
currentTime.day(), 
              currentTime.hour(), currentTime.minute(), 
currentTime.second()); 
      lastFeedTimestamp = String(datetime);  // Simpan timestamp 
pemberian pakan 
      feedTriggered = true; 
    } 
  } else { 
    feedTriggered = false; 
  } 
   
  float voltage = readSensor(pH_PIN);        // Baca rata-rata 
tegangan dari sensor pH 
  float pH = calculatePH(voltage);             // Hitung nilai pH 
berdasarkan tegangan 
  float tempC = sensors.getTempCByIndex(0);      // Baca nilai suhu 
dari sensor DS18B20 
   
  Blynk.virtualWrite(V3, pH);                  // Kirim nilai pH ke 
virtual pin V3 
   
  perintahGambar(pH, tempC);                   // Evaluasi kondisi 
untuk pengambilan gambar 
   
  // Gunakan dua servo untuk pengaturan pH 
  adjustPH(pH); 
   
  // Jika mekanisme pemberian pakan otomatis aktif, jalankan servo 
feed 
 if (feedActive) { 
    Serial.println("Aktivasi servo pakan otomatis..."); 
    feedServo.write(30);   // Buka pakan 
    delay(1000);           // Delay agar aksi terlihat 
    feedServo.write(0);    // Tutup pakan 
    feedActive = false;    // Reset flag pemberian pakan otomatis 
  } 
   
  suhu();                 // Panggil fungsi suhu untuk membaca dan 
mengirim nilai suhu ke Blynk 
  logCombinedReport(pH, tempC);  // Buat dan kirim laporan 
notifikasi ke Blynk 
   
  delay(1000);  // Delay 1 detik sebelum iterasi berikutnya 
} 