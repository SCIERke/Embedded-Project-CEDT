#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <proj_wifi.h>
#include <proj_mqtt.h>
#include <secrets.h>   // TELEGRAM_BOT_TOKEN / TELEGRAM_CHAT_ID (make firmware-secrets)

#define NOTIFY_TOPIC "notify"     // anything published here (plain text) is forwarded to Telegram
#define NOTIFY_MIN_MS 3000        // Telegram allows ~1 msg/s per chat; stay well under it
#define NOTIFY_MAX_LEN 200

String pending;                   // set by onMessage(), sent from loop(): HTTPS must not run inside the MQTT callback
uint32_t lastSent;

String urlEncode(const String &s) {
  String o;
  for (char c : s) {
    if (isalnum((unsigned char)c)) { o += c; continue; }
    char b[4];
    snprintf(b, sizeof b, "%%%02X", (uint8_t)c);
    o += b;
  }
  return o;
}

bool telegramSend(const String &text) {
  WiFiClientSecure tls;
  tls.setInsecure();   // ponytail: no certificate check; pin api.telegram.org's root CA before real deployment
  HTTPClient http;
  http.setTimeout(5000);
  if (!http.begin(tls, "https://api.telegram.org/bot" TELEGRAM_BOT_TOKEN "/sendMessage")) return false;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int code = http.POST("chat_id=" TELEGRAM_CHAT_ID "&text=" + urlEncode(text));
  http.end();
  Serial.printf("telegram -> %d\n", code);
  return code == 200;
}

void onMessage(const char *topic, const uint8_t *payload, unsigned int len) {
  pending = "";
  pending.concat((const char *)payload, min(len, (unsigned int)NOTIFY_MAX_LEN));   // latest message wins
}

void setup() {
  Serial.begin(115200);
  wifiBegin();
  mqttBegin("gateway", onMessage);
  mqttSubscribe(NOTIFY_TOPIC);
  pending = "Gateway online";
}

void loop() {
  wifiLoop();
  mqttLoop();

  // ponytail: no retry/queue; a failed send or a message arriving inside the rate window is dropped
  if (pending.length() && WiFi.status() == WL_CONNECTED && millis() - lastSent > NOTIFY_MIN_MS) {
    telegramSend(pending);
    pending = "";
    lastSent = millis();
  }
}
