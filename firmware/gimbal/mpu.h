#pragma once
#include <Arduino.h>

enum AccelRange : uint8_t { ACCEL_2G = 0x00, ACCEL_4G = 0x08, ACCEL_8G = 0x10, ACCEL_16G = 0x18 };
enum GyroRange  : uint8_t { GYRO_250 = 0x00, GYRO_500 = 0x08, GYRO_1000 = 0x10, GYRO_2000 = 0x18 };

struct ImuRaw  { int16_t ax, ay, az, temp, gx, gy, gz; };
struct ImuData { float ax, ay, az, gx, gy, gz, tempC; };
struct Angles  { float roll, pitch, yaw; };

bool mpuWriteReg(uint8_t reg, uint8_t val);
uint8_t mpuReadReg(uint8_t reg);   // returns 0xFF if the read failed
bool mpuBegin(AccelRange aRange = ACCEL_2G, GyroRange gRange = GYRO_250);
bool mpuRead(ImuData &d);
bool mpuReadRaw(ImuRaw &r);
void mpuCalibrate(uint16_t samples = 500);
void updateAngles(Angles &a, const ImuData &d, float dt, float alpha = 0.98f);
extern float gxOff, gyOff, gzOff;

// ---------------- Example ----------------
// Angles angles = {0, 0, 0};
// uint32_t lastMicros;

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(I2C_SDA, I2C_SCL);
//   Wire.setClock(400000);

//   if (!mpuBegin(ACCEL_2G, GYRO_250)) {
//     Serial.println("MPU6050 not found — check wiring");
//     while (true) delay(1000);
//   }

//   Serial.println("Calibrating... keep it still");
//   mpuCalibrate();
//   Serial.printf("Gyro offsets: %.3f %.3f %.3f\n", gxOff, gyOff, gzOff);

//   lastMicros = micros();
// }

// void loop() {
//   ImuData d;
//   if (!mpuRead(d)) {
//     Serial.println("read failed");
//     delay(100);
//     return;
//   }

//   uint32_t now = micros();
//   float dt = (now - lastMicros) * 1e-6f;
//   lastMicros = now;

//   updateAngles(angles, d, dt);

//   static uint32_t lastPrint = 0;
//   if (millis() - lastPrint > 50) {
//     lastPrint = millis();
//     Serial.printf("roll %7.2f  pitch %7.2f  yaw %7.2f\n",
//                   angles.roll, angles.pitch, angles.yaw);
//   }
// }