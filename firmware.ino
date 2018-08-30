#include "pinout.h"
#include "communication.h"
#include "pulse.h"

#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"
//extern BLESerial BLESerial; // TODO?


void sendData() {
    sendPulseData();
    // sendOrientations();  // TODO
    // sendAccelerations(); // TODO
}

void setup() {
    pinsSetup();
    serialSetup();

    // Start pulse measurements and set data TX callback function (using timers)
    pulseSetup(NULL); // NULL => use pulseDataIsReady() to know when ready

    // IMUsetup(); // TODO

    wirelessSetup(); // IMPORTANT: KEEP IT LAST!
}

void loop() {
    if (pulseDataIsReady()) { // should be called as often as possible
        sendData();
    }
}

