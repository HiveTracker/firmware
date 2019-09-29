#include "communication.h"

void setup() {
    pinsSetup();
    serialSetup();
    wirelessSetup(); // IMPORTANT: KEEP IT LAST!
}

void loop() {
    sendPulseData(); // simulated
}

