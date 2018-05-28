#include "customPinout.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
Adafruit_BNO055 bno;

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
    bno = Adafruit_BNO055();
    while(!bno.begin()) {
        Serial.println("ERROR: BNO055 not detected!"); // TODO: Use I2C ADDR / sensorID?
        digitalWrite(LEDpins[2], 1);
        delay(100);
        digitalWrite(LEDpins[2], 0);
        delay(100);
    }
    bno.setExtCrystalUse(true);

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
    delay(50);
}

// Send gravity vector on [BLE]serial ports.
void sendIMUdata() {
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

  if (BLESerial)
      BLESerial.println(euler.z());

  Serial.print(     euler.z(), 3); // 3 decimals seem sufficient
  Serial.print('\n');
}
