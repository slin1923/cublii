#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "config.h"
#include "hw/imu.h"

static Adafruit_MPU6050 mpu;

// set pins, DAQ frequency, and configure measurement ranges
bool imu_begin() {
  Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
  if (!mpu.begin(MPU6050_I2CADDR_DEFAULT, &Wire)) return false;
  Wire.setClock(IMU_I2C_HZ);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  return true;
}

// read a single sample from the IMU into the provided output struct
bool imu_read(ImuSample& out) {
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) return false;
  out.t_ms = millis();
  out.ax = a.acceleration.x; out.ay = a.acceleration.y; out.az = a.acceleration.z;
  out.gx = g.gyro.x;         out.gy = g.gyro.y;         out.gz = g.gyro.z;
  return true;
}
