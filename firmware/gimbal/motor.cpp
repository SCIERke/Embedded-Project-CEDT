#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "motor.h"

static Adafruit_PWMServoDriver pwm;  // default address 0x40

bool servoBegin() {
  Wire.beginTransmission(0x40);
  if (Wire.endTransmission() != 0) return false;  // PCA9685 not on bus
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);
  return true;
}

void servoWrite(uint8_t channel, float angle) {
  angle = constrain(angle, 0.0f, 180.0f);
  pwm.setPWM(channel, 0, servoMIN + (servoMAX - servoMIN) * angle / 180.0f);
}
