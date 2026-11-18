#pragma once
#include <Arduino.h>

typedef void (*MqttHandler)(const char *topic, const uint8_t *payload, unsigned int len);

void mqttBegin(const char *clientId, MqttHandler onMessage);  // once, after wifiBegin()
void mqttSubscribe(const char *topic);                         // re-subscribed on every reconnect
bool mqttPublish(const char *topic, const char *payload);
void mqttLoop();                                               // every loop(), never blocks
