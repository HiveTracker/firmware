#include "customPinout.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
Adafruit_BNO055 IMU;

#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"
BLESerial BLESerial;

int LEDpins[] = {10, 14, 12}; // R, G, B
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];


void setup() {
    for (int i=0; i<LEDNum; i++) {
        pinMode(LEDpins[i], OUTPUT);
        digitalWrite(LEDpins[i], 0); // 0 = ON (inverted logic)
        delay(100);
        digitalWrite(LEDpins[i], 1); // 1 = OFF (inverted logic)
    }

    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(115200);
    Serial.println("Starting Serial...");

    // Initialise the IMU
    Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
    IMU = Adafruit_BNO055();
    while(!IMU.begin()) {
        Serial.println("ERROR: BNO055 not detected!"); // TODO: Use I2C ADDR / sensorID?
        digitalWrite(LEDpins[2], 1);
        delay(100);
        digitalWrite(LEDpins[2], 0);
        delay(100);
    }

    BLESerial.setLocalName("UART");
    BLESerial.begin();
}

void loop() {
    static bool state = 0;
    BLESerial.poll();

    sendIMUdata();

    for (int i=0; i<LEDNum; i++)
        digitalWrite(LEDpins[i], state);
    state = !state;
    delay(33); // 30Hz
}

// Send orientation vectors on [BLE]serial ports.
void sendIMUdata() {

    const bool sendQuaternions = true;

    if (sendQuaternions) {

        imu::Quaternion quat = IMU.getQuat();

        if (BLESerial) {
            BLESerial.print(quat.w()); BLESerial.print('\t');
            BLESerial.print(quat.x()); BLESerial.print('\t');
            BLESerial.print(quat.y()); BLESerial.print('\t');
            BLESerial.print(quat.z()); BLESerial.print('\n');
        }
        Serial.print(quat.w()); Serial.print('\t');
        Serial.print(quat.x()); Serial.print('\t');
        Serial.print(quat.y()); Serial.print('\t');
        Serial.print(quat.z()); Serial.print('\n');

    } else {

        imu::Vector<3> euler = IMU.getVector(Adafruit_BNO055::VECTOR_EULER);

        if (BLESerial) {
            BLESerial.print(euler.x()); BLESerial.print('\t');
            BLESerial.print(euler.y()); BLESerial.print('\t');
            BLESerial.print(euler.z()); BLESerial.print('\n');
        }
        Serial.print(euler.x()); Serial.print('\t');
        Serial.print(euler.y()); Serial.print('\t');
        Serial.print(euler.z()); Serial.print('\n');

    }
}

