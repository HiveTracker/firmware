#include "pulse.h"
// #include "communication.h" // TODO
#include "pinout.h" // important to keep it last for the undefs to work

extern void sendPulseData();

void sendData() {
    sendPulseData();
    // sendOrientations();  // TODO
    // sendTXend();         // TODO
}

void setup() {
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);

    // communicationSetup(); // TODO
    // IMUsetup(); // TODO

    // Start pulse measurements and set data TX callback function (using timers)
    pulseSetup(sendData);
}

void loop() { }


