#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include "imu.h"
#include "pinout.h"
Adafruit_BNO055 IMU;

int LEDpins[] = {10, 14, 12}; // R, G, B
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];
imu::Vector<3> accel;

void IMUsetup() {
    Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
    IMU = Adafruit_BNO055();
    while(!IMU.begin()) {
        Serial.println("ERROR: BNO055 not detected!");
    }
    IMU.setExtCrystalUse(true);
}


void fetchIMUdata() {
    // anticipate data sync for next TX:
    accel = IMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
}


// Send orientation vectors IN BINARY (!!!) on serial port.
void sendAccelerations() {
    // https://github.com/adafruit/Adafruit_BNO055/blob/master/Adafruit_BNO055.cpp#L359-L361

    double gravity = 9.81;

    for (int i=0; i<3; i++) {
        accel[i] += gravity * 2.0;              // max negative value = -2g
        accel[i] *= (1<<15) / (4.0 * gravity);  // normalize with max amplitude

        uint16_t buf = (accel[i] < 0)? 0 : uint16_t(accel[i]); // stay positive
        Serial.write((buf >> 0) & 0xFF); // LSB first
        Serial.write((buf >> 8) & 0xFF); // MSB last
    }
}


// Send orientation vectors IN ASCII (!!!) on serial port.
void sendOrientations() {
    const bool sendQuaternions = true;

    if (sendQuaternions) {
        imu::Quaternion quat = IMU.getQuat();
        Serial.print(quat.w()); Serial.print('\t');
        Serial.print(quat.x()); Serial.print('\t');
        Serial.print(quat.y()); Serial.print('\t');
        Serial.print(quat.z()); Serial.print('\n');
    } else {
        imu::Vector<3> euler = IMU.getVector(Adafruit_BNO055::VECTOR_EULER);
        Serial.print(euler.x()); Serial.print('\t');
        Serial.print(euler.y()); Serial.print('\t');
        Serial.print(euler.z()); Serial.print('\n');
    }
}


