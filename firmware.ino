#include "pulse.h"
#include "pinout.h" // important to keep it last for the undefs to work

void setup() {
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);

    pinMode(PIN_SERIAL_RX, OUTPUT);

    pulseSetup(); // Start pulse measurements...
}

void loop() { }


