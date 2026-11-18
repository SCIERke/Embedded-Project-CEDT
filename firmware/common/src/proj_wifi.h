#pragma once
#include <Arduino.h>

// Blocks up to timeoutMs (use in setup() only). Returns true if connected.
bool wifiBegin(uint32_t timeoutMs = 10000);

// Call every loop(). Never blocks: if the link dropped, kicks off a reconnect every few seconds.
void wifiLoop();
