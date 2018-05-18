#include "pinout.h"
#include "pulse.h"
#include "PPI.h"
#include "Timer.h"
#include "nRF_SDK/nrf_timer.h"

void setup() {
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);

    pinMode(PIN_SERIAL_RX, OUTPUT);

    pulseSetup(); // Start pulse measurements...
}

void loop() { }


