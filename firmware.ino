#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"

// create ble serial instance, see pinouts above
BLESerial BLESerial(0,0,0);         // TODO: use constructor default values

// LED pins
int LEDpins[] = {17, 18, 19, 20};   // TODO with HiveTracker: 10, 12, 14
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];
bool state = 0;

void setup() {
    // custom services and characteristics can be added as well
    BLESerial.setLocalName("UART");

    Serial.begin(115200);
    BLESerial.begin();

    for (int i=0; i<LEDNum; i++) {
        pinMode(LEDpins[i], OUTPUT);
        digitalWrite(LEDpins[i], 0); // inverted logic
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
