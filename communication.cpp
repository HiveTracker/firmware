#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"

#include "pulse.h"
#include "pinout.h" // important to keep it last for the undefs to work
#include "communication.h"
#include "firmware.h"

byte message[10];
BLESerial bleSerial;
#define BLE_NAME "HT-UART"

int LEDpins[] = {14, 12}; // G, B
int LEDNum = sizeof LEDpins / sizeof LEDpins[0];

void pinsSetup()
{
    // LEDs & debug pins
    for (int i = 0; i < LEDNum; i++)
    {
        pinMode(LEDpins[i], OUTPUT);
        digitalWrite(LEDpins[i], 0); // 0 = ON (inverted logic)
        delay(100);
        digitalWrite(LEDpins[i], 1); // 1 = OFF (inverted logic)
    }

    pinMode(PIN_SERIAL_RX, OUTPUT); // TODO REMOVE (TMP)
}

void serialSetup()
{
    Serial.setPins(0, PIN_SERIAL_TX); // RX is not used here
    Serial.begin(230400);
    Serial.println("Serial OK!");
}

void wirelessSetup()
{
    Serial.println("Starting BLE-UART...");
    bleSerial.setLocalName(BLE_NAME);
    bleSerial.setDeviceName(BLE_NAME);
    bleSerial.setConnectionInterval(0x0006, 0x0006);
    bleSerial.begin();
}

void sendPulseData()
{
    // Send binary metadata (base ID and axis)
    uint8_t base_axis = pulse_data.baseID << 1 | pulse_data.axis;
    message[0] = base_axis;
    message[1] = 0;
    for (int t = 0, i = 0; t < 2; t++)
    {
        // send captures
        for (int c = 0; c < sensors_num; c += 2, i++)
        {
            int pulseStart = pulse_data.sweep_captures[t][c];
            int pulseEnd = pulse_data.sweep_captures[t][c + 1];
            int pulseWidthTicks16 = pulseEnd - pulseStart;
            // TODO: deport data verification to higher level?
            if (pulseWidthTicks16 < minSweepPulseWidth || // ticks
                pulseWidthTicks16 > maxSweepPulseWidth || // ticks
                pulseStart < sweepStartTicks ||           // ticks
                pulseEnd > sweepEndTicks)                 // ticks
            {
                // mark the measures if they are invalid
                pulseStart = 0;
                pulseEnd = 0;
            }

            // get centroid + remove 2 LSb (non-significant) to stay in 16bit
            // TODO: use differential loss-less compression
            int centroid = ((pulseEnd + pulseStart) / 2) >> 2;
            message[i * 2 + 2] = (centroid >> 8) & 0xFF; // MSB first
            message[i * 2 + 3] = (centroid >> 0) & 0xFF; // LSB last
            message[1] += centroid & 0xFF;               // add LSB to checksum
        }
    }

    // set the high-bits and metadata on message separator (base/axis + checksum)
    message[0] = 0x80 | (message[0] << 5) & 0x60 | (message[1] >> 4) & 0x0F;
    message[1] = 0x80 | (message[1] >> 0) & 0x0F;
    if (bleSerial) {
        bleSerial.write(message, 10);
        bleSerial.poll();
    }
    if (Serial)
        Serial.write(message, 10);
}
