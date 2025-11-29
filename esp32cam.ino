// KODINGAN ESP 32 CAM 
#include <WiFi.h> 
#include "ESP32_MailClient.h" 
#include <HTTPClient.h> 
#include "esp_camera.h" 
 
// WiFi Credentials 
const char* ssid = "🍎99"; 
const char* password = "1234567***"; 
 
// API Endpoint 
const char* api_url = "https://databasesih.com/api/camera"; // Ganti 
dengan URL API kamu 
const char* upd_off = "https://databasesih.com/api/camera/0"; // 
Ganti dengan URL API kamu 
 
// Email Configuration (SMTP) 
#define SMTP_SERVER "mail.databasesih.com" 
#define SMTP_PORT 465 
#define EMAIL_SENDER "admin@databasesih.com" 
#define EMAIL_PASSWORD "Rahasia123!!" 
#define EMAIL_RECIPIENT "roysihan00@gmail.com" 
// SMTP Client 
 
// Kamera Configuration 
#define PWDN_GPIO     32 
#define RESET_GPIO    -1 
#define XCLK_GPIO     0 
#define SIOD_GPIO     26 
#define SIOC_GPIO     27 
#define Y9_GPIO       35 
#define Y8_GPIO       34 
#define Y7_GPIO       39 
#define Y6_GPIO       36 
#define Y5_GPIO       21 
#define Y4_GPIO       19 
#define Y3_GPIO       18 
#define Y2_GPIO       5 
#define VSYNC_GPIO    25 
#define HREF_GPIO     23 
#define PCLK_GPIO     22 
 
SMTPData smtpData; 
 
void setupCamera() { 
    camera_config_t config; 
    config.ledc_channel = LEDC_CHANNEL_0; 
    config.ledc_timer = LEDC_TIMER_0; 
    config.pin_d0 = Y2_GPIO; 
    config.pin_d1 = Y3_GPIO; 
    config.pin_d2 = Y4_GPIO; 
    config.pin_d3 = Y5_GPIO; 
    config.pin_d4 = Y6_GPIO; 
    config.pin_d5 = Y7_GPIO; 
    config.pin_d6 = Y8_GPIO; 
    config.pin_d7 = Y9_GPIO; 
    config.pin_xclk = XCLK_GPIO; 
    config.pin_pclk = PCLK_GPIO; 
    config.pin_vsync = VSYNC_GPIO; 
    config.pin_href = HREF_GPIO; 
    config.pin_sscb_sda = SIOD_GPIO; 
    config.pin_sscb_scl = SIOC_GPIO; 
    config.pin_pwdn = PWDN_GPIO; 
    config.pin_reset = RESET_GPIO; 
    config.xclk_freq_hz = 20000000; 
    config.pixel_format = PIXFORMAT_JPEG; 
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 12; 
    config.fb_count = 1; 
 
 
    esp_err_t err = esp_camera_init(&config); 
    if (err != ESP_OK) { 
 Serial.println("Kamera gagal diinisialisasi"); 
        return; 
    } 
} 
 
bool checkAPI() { 
    HTTPClient http; 
    http.begin(api_url); 
    int httpCode = http.GET(); 
     
    if (httpCode > 0) { 
        String payload = http.getString(); 
        Serial.println("Response API: " + payload); 
         
        if (payload.indexOf("\"capture\":1") > 0) { 
            http.end(); 
            return true; 
        } 
    } 
     
    http.end(); 
    return false; 
} 
 
void updOff() { 
    HTTPClient http; 
    http.begin(upd_off); 
    int httpCode = http.GET(); 
     
    if (httpCode > 0) { 
         
        Serial.println("Data Capture Ter-reset"); 
    } 
     
    http.end(); 
} 
 
void sendEmail(camera_fb_t *fb) { 
   
  smtpData.setLogin(SMTP_SERVER, SMTP_PORT, EMAIL_SENDER, 
EMAIL_PASSWORD); 
  smtpData.setSender("ESP32-CAM", EMAIL_SENDER); 
  smtpData.setPriority("High"); 
  smtpData.setSubject("Foto dari ESP32-CAM"); 
  smtpData.setMessage("Foto dari ESP32-CAM", true); 
  smtpData.addRecipient(EMAIL_RECIPIENT);
   // Membuat objek String untuk nama file dan tipe MIME 
  String fileName = "image.jpg"; 
  String mimeType = "image/jpeg"; 
 
  // Mengirimkan gambar yang diambil langsung melalui lampiran 
  smtpData.addAttachData(fileName, mimeType, fb->buf, fb->len); 
 
  // Kirim email 
  if (!MailClient.sendMail(smtpData)) { 
    Serial.println("Gagal mengirim email"); 
  } else { 
    Serial.println("Email berhasil dikirim!"); 
  } 
 
  // Hapus data email 
  smtpData.empty(); 
 
} 
 
void setup() { 
    Serial.begin(115200); 
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) { 
        delay(1000); 
        Serial.print("."); 
    } 
    Serial.println("WiFi terhubung!"); 
 
    setupCamera(); 
} 
 
void loop() { 
    if (checkAPI()) { 
        Serial.println("Capture = 1, mengambil gambar..."); 
         
        camera_fb_t *fb = esp_camera_fb_get(); 
        if (!fb) { 
            Serial.println("Gagal mengambil gambar"); 
            return; 
        } 
        sendEmail(fb); 
        esp_camera_fb_return(fb); 
        updOff(); 
    } 
 
    delay(10000); // Cek API setiap 5 detik