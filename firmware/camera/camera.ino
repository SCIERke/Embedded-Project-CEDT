#include <WebServer.h>
#include <esp_camera.h>
#include <proj_wifi.h>

#define SERIAL_BAUDRATE 115200

// ---- TODO: replace with your OV7670 wiring (these are the AI-Thinker ESP32-CAM pins) ----
#define PIN_PWDN  32
#define PIN_RESET -1
#define PIN_XCLK   0
#define PIN_SIOD  26
#define PIN_SIOC  27
#define PIN_D0     5   // OV7670 D0..D7 -> Y2..Y9
#define PIN_D1    18
#define PIN_D2    19
#define PIN_D3    21
#define PIN_D4    36
#define PIN_D5    39
#define PIN_D6    34
#define PIN_D7    35
#define PIN_VSYNC 25
#define PIN_HREF  23
#define PIN_PCLK  22

// ponytail: OV7670 has no JPEG engine, so we grab RGB565 and software-encode (a few FPS at QVGA).
// Upgrade to an OV2640 (hardware JPEG) if the tracker needs more FPS.
static bool cameraBegin() {
  camera_config_t c = {};
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = PIN_D0; c.pin_d1 = PIN_D1; c.pin_d2 = PIN_D2; c.pin_d3 = PIN_D3;
  c.pin_d4 = PIN_D4; c.pin_d5 = PIN_D5; c.pin_d6 = PIN_D6; c.pin_d7 = PIN_D7;
  c.pin_xclk = PIN_XCLK; c.pin_pclk = PIN_PCLK; c.pin_vsync = PIN_VSYNC; c.pin_href = PIN_HREF;
  c.pin_sccb_sda = PIN_SIOD; c.pin_sccb_scl = PIN_SIOC;
  c.pin_pwdn = PIN_PWDN; c.pin_reset = PIN_RESET;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = PIXFORMAT_RGB565;
  c.frame_size = FRAMESIZE_QVGA;   // 320x240, plenty for YOLO at 320px
  c.fb_count = 1;
  return esp_camera_init(&c) == ESP_OK;
}

WebServer server(81);

void handleStream() {
  WiFiClient client = server.client();
  client.print("HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=f\r\n\r\n");
  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;
    uint8_t *jpg = nullptr;
    size_t len = 0;
    bool ok = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, fb->format, 80, &jpg, &len);
    esp_camera_fb_return(fb);
    if (!ok) continue;
    client.printf("--f\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)len);
    client.write(jpg, len);
    client.print("\r\n");
    free(jpg);
  }
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  if (!cameraBegin()) {
    Serial.println("OV7670 init failed — check wiring / pin map / OV7670 support in esp32-camera");
    while (true) delay(1000);
  }
  if (!wifiBegin()) Serial.println("WiFi not connected yet, will keep retrying");
  server.on("/stream", handleStream);
  server.begin();
  Serial.printf("Stream: http://%s:81/stream\n", WiFi.localIP().toString().c_str());
}

void loop() {
  wifiLoop();
  server.handleClient();
}
