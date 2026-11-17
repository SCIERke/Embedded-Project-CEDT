#include <Wire.h>
#include <Arduino.h>
#include "mpu.h"

// ---------------- Config ----------------
#define I2C_SDA      21
#define I2C_SCL      22
#define MPU_ADDR     0x68      // 0x69 if AD0 pin is HIGH

// ---------------- Registers ----------------
#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_XOUT_H 0x3B
#define REG_PWR_MGMT_1   0x6B
#define REG_WHO_AM_I     0x75

static float accelLsbPerG  = 16384.0f;
static float gyroLsbPerDps = 131.0f;
static float axOff = 0, ayOff = 0, azOff = 0;
static float gxOff = 0, gyOff = 0, gzOff = 0;

// Returns true if the MPU ACKed the write (endTransmission == 0).
bool mpuWriteReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission(true) == 0;
}

// Returns the register value, or 0xFF if the MPU did not answer.
uint8_t mpuReadReg(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0xFF;   // repeated start
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)1) != 1) return 0xFF;
  return Wire.read();
}

// ------ Reading ------
bool mpuReadRaw(ImuRaw &r) {
  uint8_t b[14];
  if (!mpuReadRegs(REG_ACCEL_XOUT_H, b, 14)) return false;
  r.ax   = (int16_t)(b[0]  << 8 | b[1]);
  r.ay   = (int16_t)(b[2]  << 8 | b[3]);
  r.az   = (int16_t)(b[4]  << 8 | b[5]);
  r.temp = (int16_t)(b[6]  << 8 | b[7]);
  r.gx   = (int16_t)(b[8]  << 8 | b[9]);
  r.gy   = (int16_t)(b[10] << 8 | b[11]);
  r.gz   = (int16_t)(b[12] << 8 | b[13]);
  return true;
}

bool mpuRead(ImuData &d) {
  ImuRaw r;
  if (!mpuReadRaw(r)) return false;
  d.ax = r.ax / accelLsbPerG  - axOff;
  d.ay = r.ay / accelLsbPerG  - ayOff;
  d.az = r.az / accelLsbPerG  - azOff;
  d.gx = r.gx / gyroLsbPerDps - gxOff;
  d.gy = r.gy / gyroLsbPerDps - gyOff;
  d.gz = r.gz / gyroLsbPerDps - gzOff;
  d.tempC = r.temp / 340.0f + 36.53f;
  return true;
}

// ---------------- Calibration (keep still & flat) ----------------
void mpuCalibrate(uint16_t samples = 500) {
  axOff = ayOff = azOff = gxOff = gyOff = gzOff = 0;
  double sax = 0, say = 0, saz = 0, sgx = 0, sgy = 0, sgz = 0;
  ImuData d;
  for (uint16_t i = 0; i < samples; i++) {
    mpuRead(d);
    sax += d.ax; say += d.ay; saz += d.az;
    sgx += d.gx; sgy += d.gy; sgz += d.gz;
    delay(2);
  }
  axOff = sax / samples;
  ayOff = say / samples;
  azOff = saz / samples - 1.0f;   // Z should read +1 g
  gxOff = sgx / samples;
  gyOff = sgy / samples;
  gzOff = sgz / samples;
}

// ---------------- Complementary filter ----------------
void updateAngles(Angles &a, const ImuData &d, float dt, float alpha = 0.98f) {
  float accRoll  = atan2f(d.ay, d.az) * RAD_TO_DEG;
  float accPitch = atan2f(-d.ax, sqrtf(d.ay * d.ay + d.az * d.az)) * RAD_TO_DEG;
  a.roll  = alpha * (a.roll  + d.gx * dt) + (1 - alpha) * accRoll;
  a.pitch = alpha * (a.pitch + d.gy * dt) + (1 - alpha) * accPitch;
  a.yaw  += d.gz * dt;   // drifts, no reference
}

// ------ Setup ------ (call Wire.begin() first)
bool mpuBegin(AccelRange aRange, GyroRange gRange) {
  uint8_t who = mpuReadReg(REG_WHO_AM_I);
  if (who != 0x68) {
    Serial.printf("WHO_AM_I = 0x%02X (expected 0x68)\n", who);
    if (who == 0x00 || who == 0xFF) return false;   // nothing answered; other values = clones, carry on
  }
  if (!mpuWriteReg(REG_PWR_MGMT_1, 0x01)) return false;   // wake up, PLL clock
  delay(100);

  if (!mpuWriteReg(REG_ACCEL_CONFIG, aRange)) return false;
  if (!mpuWriteReg(REG_GYRO_CONFIG, gRange)) return false;
  delay(20);
  return true;
}
