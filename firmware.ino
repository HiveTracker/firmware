#include "customPinout.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
Adafruit_BNO055 bno;

#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"
BLESerial BLESerial(0,0,0);         // TODO: use constructor default values

int LEDpins[] = {10, 14, 12}; // R, G, B
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];
bool state = 0;


void setup() {
    BLESerial.setLocalName("UART");

    Serial.begin(115200);
    BLESerial.begin();

    for (int i=0; i<LEDNum; i++) {
        pinMode(LEDpins[i], OUTPUT);
        digitalWrite(LEDpins[i], 0); // 0 = ON (inverted logic)
        delay(300);
        digitalWrite(LEDpins[i], 1); // 1 = OFF (inverted logic)
    }

    // Initialise the IMU
    Wire.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
    bno = Adafruit_BNO055();
    while(!bno.begin()) {
        Serial.print("ERROR No BNO055 detected!"); // TODO: Use I2C ADDR / sensorID?

        digitalWrite(12, 1);
    }
}

void loop() {
    BLESerial.poll();

    forward();
    loopback();
    spam();

    for (int i=0; i<LEDNum; i++)
        digitalWrite(LEDpins[i], state);
    state = !state;
    delay(50);
}


// forward received from Serial to BLESerial and vice versa
void forward() {
    if (BLESerial && Serial) {
        int byte;
        while ((byte = BLESerial.read()) > 0) Serial.write((char)byte);
        while ((byte = Serial.read()) > 0) BLESerial.write((char)byte);
    }
}

// echo all received data back
void loopback() {
    if (BLESerial) {
        int byte;
        while ((byte = BLESerial.read()) > 0) BLESerial.write(byte);
    }
}

// periodically sent time stamps
void spam() {
    if (BLESerial) {
        BLESerial.print(millis());
        BLESerial.println(" tick-tacks!");
        delay(100);

    }
}

