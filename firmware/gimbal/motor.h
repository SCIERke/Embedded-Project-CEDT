#pragma once
#include <stdint.h>

// Pulse ticks out of 4096 at SERVO_FREQ. Tune per servo (see README/hardware notes).
#ifndef servoMIN
#define servoMIN 150
#endif
#ifndef servoMAX
#define servoMAX 1000
#endif
#ifndef SERVO_FREQ
#define SERVO_FREQ 60
#endif

bool servoBegin();
void servoWrite(uint8_t channel, float angle);  // angle 0..180, clamped
