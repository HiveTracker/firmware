#include <SPI.h> // BLEPeripheral depends on SPI
#include <BLEPeripheral.h>
#include "BLESerial.h"

#include "pinout.h" // important to keep it last for the undefs to work
#include "communication.h"
#include "firmware.h"

#include "simulation.h" // contains raw data recorded from simulation app
// it's generated with: xxd -i data.bin > simulation.h
// this command creates data_bin[] and data_bin_len
// max data size is about 370KB


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
    static int offset = 0;

    if (bleSerial) {
        bleSerial.poll();
        int bytesSent = bleSerial.write(data_bin + offset, 10);

        offset += bytesSent;
        if (offset + 10 >= data_bin_len) // anticipate next 10 bytes message
            offset = 0;
    }

    delayMicroseconds(8333); // 120Hz
}
