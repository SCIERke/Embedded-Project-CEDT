#include <WiFi.h>
#include <PubSubClient.h>
#include "proj_mqtt.h"
#include "secrets.h"   // MQTT_HOST/PORT/USER/PASSWORD

#define N_LIMITED_TOPICS 4

static WiFiClient net;
static PubSubClient client(net);
static const char *id;
static const char *topics[N_LIMITED_TOPICS];
static int nTopics = 0;

void mqttBegin(const char *clientId, MqttHandler onMessage) {
  id = clientId;
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback([onMessage](char *t, uint8_t *p, unsigned int n) { onMessage(t, p, n); });
}

void mqttSubscribe(const char *topic) {
  if (nTopics < N_LIMITED_TOPICS) topics[nTopics++] = topic;
  if (client.connected()) client.subscribe(topic);
}

bool mqttPublish(const char *topic, const char *payload) {
  return client.connected() && client.publish(topic, payload);
}

void mqttLoop() {
  static uint32_t lastTry = 0;
  if (WiFi.status() != WL_CONNECTED) return;
  if (!client.connected()) {
    if (millis() - lastTry < 5000) return;
    lastTry = millis();
    bool ok = MQTT_USER[0] ? client.connect(id, MQTT_USER, MQTT_PASSWORD) : client.connect(id);
    if (!ok) return;
    for (int i = 0; i < nTopics; i++) client.subscribe(topics[i]);
  }
  client.loop();
}
