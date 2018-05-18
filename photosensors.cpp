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
    for (int i = 0; i < sensors_num; i++) {
        if (devices[i].waitForLight(light_timeout)) {
            DEBUG_PRINTLN("LD"); // Light Detected

            uint8_t config_result = devices[i].configDevice();

            switch (config_result) {
                case CONFIG_PASS:
                    DEBUG_PRINTLN("OK");
                    break;
                case BUS_FAIL:
                    // unable to resolve state of TS4231 (3 samples of the bus
                    // signals resulted in 3 different states)
                    DEBUG_PRINTLN("ERROR: BUS_FAIL");
                    break;
                case VERIFY_FAIL:
                    // configuration read value did not match configuration write
                    // value, run configuration again
                    DEBUG_PRINTLN("ERROR: VERIFY_FAIL");
                    break;
                case WATCH_FAIL:
                    // verify succeeded but entry into WATCH mode failed, run
                    // configuration again
                    DEBUG_PRINTLN("ERROR: WATCH_FAIL");
                    break;
                default:  //value returned was unknown
                    DEBUG_PRINTLN("ERROR: unknown value");
                    break;
            }
        }
        else {
            //insert code here for no light detection
            DEBUG_PRINTLN("TIMEOUT");
        }
    }
}

