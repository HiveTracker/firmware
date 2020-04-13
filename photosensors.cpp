#include "pinout.h"
#include "ts4231.h"
#include "photosensors.h"
#include "firmware.h"

#define light_timeout 500 // 500ms is a placeholder - system dependent


TS4231 devices[] = { TS4231(sensors_e[0], sensors_d[0]),
                     TS4231(sensors_e[1], sensors_d[1]),
                     TS4231(sensors_e[2], sensors_d[2]),
                     TS4231(sensors_e[3], sensors_d[3]) };

void photosensorSetup() {
    uint8_t config_result = -1;

    for (int i = 0; i < sensors_num; i++) {
        bool exit = 0;

        while (!exit) {
            DEBUG_PRINTLN(i);
            if (!devices[i].waitForLight(light_timeout)) {
                DEBUG_PRINTLN("TIMEOUT");
                static bool state = 0;
                digitalWrite(LED_G, state = !state); // display problem
                continue;
            }

            DEBUG_PRINTLN("LD"); // Light Detected

            config_result = devices[i].configDevice();

            switch (config_result) {
                case CONFIG_PASS:
                    exit = 1;
                    DEBUG_PRINTLN("OK");
                    break;
                case BUS_FAIL:
                    // unable to resolve state of TS4231 (3 samples of the bus
                    // signals resulted in 3 different states)
                    exit = 1;
                    DEBUG_PRINTLN("ERROR: BUS_FAIL");
                    break;
                case VERIFY_FAIL:
                    // configuration read value did not match configuration write
                    // value, run configuration again
                    exit = 0;
                    digitalWrite(LED_B, !digitalRead(LED_B)); // toggle
                    DEBUG_PRINTLN("ERROR: VERIFY_FAIL");
                    break;
                case WATCH_FAIL:
                    // verify succeeded but entry into WATCH mode failed, run
                    // configuration again
                    exit = 0;
                    digitalWrite(LED_G, !digitalRead(LED_G)); // toggle
                    DEBUG_PRINTLN("ERROR: WATCH_FAIL");
                    break;
                default:  //value returned was unknown
                    exit = 1;
                    DEBUG_PRINTLN("ERROR: unknown value");
                    break;
            }
        }
    }

    if (config_result != CONFIG_PASS) {
        DEBUG_PRINTLN("TS4231 BUS_FAIL -> Let's assume we use the TS4112");
    }

    // Turn off LED warnings (inverted logic)
    digitalWrite(LED_G, 1);
    digitalWrite(LED_B, 1);
    DEBUG_PRINTLN("EVERYTHING OK");
}

