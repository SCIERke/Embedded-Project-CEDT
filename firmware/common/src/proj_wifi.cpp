#include <WiFi.h>
#include "proj_wifi.h"
#include "secrets.h"   // generated from .env by `make firmware-secrets`

#define WIFI_RETRY_MS 5000

bool wifiBegin(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeoutMs) delay(100);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    return false;
  }
  Serial.printf("WiFi connected, IP %s\n", WiFi.localIP().toString().c_str());
  return true;
}

void wifiLoop() {
  static uint32_t lastTry = 0;
  if (WiFi.status() == WL_CONNECTED || millis() - lastTry < WIFI_RETRY_MS) return;
  lastTry = millis();
  WiFi.reconnect();
}
