#include "pulse.h"
#include "imu.h"
#include "pinout.h" // important to keep it last for the undefs to work

extern void sendPulseData();
extern void sendAccelerations();
bool readyToFetch = false;

void sendData() {
    sendPulseData();
    sendAccelerations();
    // sendOrientations();  // TODO
    readyToFetch = true;
}

void setup() {
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);
    Serial.println("Starting...");

    pinMode(PIN_SERIAL_RX, OUTPUT); // TODO REMOVE (tmp)

    IMUsetup();

    // Start pulse measurements and set data TX callback function (using timers)
    pulseSetup(sendData);
}

void loop() {
    if (readyToFetch) {
        digitalWrite(PIN_SERIAL_RX, 1);
        // anticipate data sync for next TX:
        fetchIMUdata(); // can take about 2ms
        digitalWrite(PIN_SERIAL_RX, 0);
        readyToFetch = false;
    }
}

