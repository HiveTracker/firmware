#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"
BLESerial BLESerial;

#include "pulse.h"
#include "pinout.h" // important to keep it last for the undefs to work
#include "communication.h"
#include "firmware.h"

int LEDpins[] = {14, 12}; // G, B
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];


void pinsSetup() {
    // LEDs & debug pins
    for (int i=0; i<LEDNum; i++) {
        pinMode(LEDpins[i], OUTPUT);
        digitalWrite(LEDpins[i], 0); // 0 = ON (inverted logic)
        delay(100);
        digitalWrite(LEDpins[i], 1); // 1 = OFF (inverted logic)
    }

    pinMode(PIN_SERIAL_RX, OUTPUT); // TODO REMOVE (TMP)
}

void serialSetup() {
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);
    while (!Serial); // wait
    Serial.println("Serial OK!");
}

void wirelessSetup() {
    Serial.println("Connect to BLE-UART...");
    BLESerial.setLocalName("BLE-UART");
    BLESerial.begin();
    // while (!BLESerial); // wait
    Serial.println("BLE OK!");
}

void sendPulseData() {
    // Send binary data:

    DEBUG_WRITE(0xFF); DEBUG_WRITE(0xFF);         // send start headers

    uint8_t base_axis = pulse_data.baseID << 1  |  pulse_data.axis;
    DEBUG_WRITE(base_axis);                        // send base ID and axis
    if (BLESerial)
        BLESerial.print(base_axis);

    for (int t = 0; t < 2; t++) {                   // send captures
        for (int c = 0; c < sensors_num; c += 2) {

            // Invalid pulse by default
            int pulseStart = pulse_data.sweep_captures[t][c];
            int pulseEnd = pulse_data.sweep_captures[t][c+1];
            int pulseWidthTicks16 = pulseEnd - pulseStart;

            if ( pulseWidthTicks16 < minSweepPulseWidth ||     // ticks
                 pulseWidthTicks16 > maxSweepPulseWidth ||     // ticks
                 pulseStart < sweepStartTicks           ||     // ticks
                 pulseEnd   > sweepEndTicks ) {                // ticks

                // mark the measures if they are invalid
                pulseStart *= -1;
                pulseEnd *= -1;
            }

            // get centroid + remove 2 LSb (non-significant) to stay in 16bit
            int centroid = ((pulseEnd+pulseStart)/2) >> 2;

            DEBUG_WRITE((centroid >> 0) & 0xFF);    // LSB first
            DEBUG_WRITE((centroid >> 8) & 0xFF);    // MSB last

            if (BLESerial) {
                BLESerial.print(' ');
                BLESerial.print((centroid >> 0) & 0xFF);    // LSB first
                BLESerial.print((centroid >> 8) & 0xFF);    // MSB last
            }
        }
    }
    if (BLESerial)
        BLESerial.print('\n');
}

