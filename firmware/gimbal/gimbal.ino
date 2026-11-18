#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "src/mpu.h"
#include "src/motor.h"
#include <proj_wifi.h>

#define SERIAL_BAUDRATE 115200
#define WIRE_CLOCK 400000

// ---- Tuning knobs ----
// PCA9685 channel per axis, mechanical direction (+1/-1), servo trim (deg, level = 90 + trim)
struct Axis { uint8_t ch; float dir, trim; };
Axis axRoll  = {0, +1, 0};
Axis axPitch = {1, +1, 0};
Axis axYaw   = {2, +1, 0};

float Kp = 1.5f, Ki = 0.0f, Kd = 0.05f;
const float I_MAX = 30.0f;    // anti-windup clamp (deg of servo travel)
const float OUT_MAX = 60.0f;  // servo swing limit around centre (deg)

float iRoll, iPitch, iYaw;   // integral term per axis

Angles angles = {0, 0, 0};
uint32_t lastMicros;

void pid(float &i, const Axis &ax, float angle, float rate, float dt) {
  float err = 0.0f - angle;
  i = constrain(i + Ki * err * dt, -I_MAX, I_MAX);
  float u = constrain(Kp * err + i - Kd * rate, -OUT_MAX, OUT_MAX);
  servoWrite(ax.ch, 90.0f + ax.trim + ax.dir * u);
}

void handleSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();

  if (c == 'r') {
    angles = {0, 0, 0};
    iRoll = iPitch = iYaw = 0;
    return;
  }

  if (c != 'p' && c != 'i' && c != 'd') return;   // skips '\n': parseFloat would block 1 s waiting for a number
  float v = Serial.parseFloat();

  if (c == 'p') Kp = v; else if (c == 'i') Ki = v; else Kd = v;

  Serial.printf("Kp %.3f Ki %.3f Kd %.3f\n", Kp, Ki, Kd);
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Wire.begin();
  Wire.setClock(WIRE_CLOCK);

  Serial.println("Starting up MPU... keep it still");
  if (!mpuBegin(ACCEL_2G, GYRO_250)) {
    Serial.println("MPU6050 not found — check wiring");
    while (true) delay(1000);
  }

  Serial.println("Starting up servo...");
  if (!servoBegin()) {
    Serial.println("PCA9685 not found — check wiring");
    while (true) delay(1000);
  }
  for (Axis *a : {&axRoll, &axPitch, &axYaw}) servoWrite(a->ch, 90 + a->trim);

  Serial.println("Calibrating... keep it still");
  mpuCalibrate();
  Serial.printf("Gyro offsets: %.3f %.3f %.3f\n", gxOff, gyOff, gzOff);

  Serial.println("Connecting to wifi...");
  wifiBegin();   // result ignored on purpose: wifiLoop() retries, balancing doesn't need wifi

  lastMicros = micros();
}

void loop() {
  handleSerial();
  wifiLoop();

  ImuData d;
  if (!mpuRead(d)) {
    Serial.println("read failed");
    delay(100);
    lastMicros = micros();
    return;
  }

  uint32_t now = micros();
  float dt = (now - lastMicros) * 1e-6f;
  lastMicros = now;

  updateAngles(angles, d, dt);

  pid(iRoll,   axRoll,  angles.roll,  d.gx, dt);
  pid(iPitch, axPitch, angles.pitch, d.gy, dt);
  pid(iYaw,     axYaw,   angles.yaw,   d.gz, dt);

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 50) {
    lastPrint = millis();
    Serial.printf("roll %7.2f  pitch %7.2f  yaw %7.2f\n", angles.roll, angles.pitch, angles.yaw);
  }
}
