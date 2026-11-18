#include <proj_wifi.h>

void setup() {
  wifiBegin();
  mqttBegin();
}

void loop() {
  wifiLoop();
  mqttLoop();
}