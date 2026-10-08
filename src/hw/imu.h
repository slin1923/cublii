#pragma once
#include <stdint.h>

// Units follow the Adafruit library: accel in m/s^2, gyro in rad/s.
struct ImuSample {
  uint32_t t_ms;
  float ax, ay, az;
  float gx, gy, gz;
};

bool imu_begin();                 // false if the MPU6050 isn't found
bool imu_read(ImuSample& out);
