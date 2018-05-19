#include <math.h>
#include "pulse.h"
#include "pinout.h"
#include "PPI.h"
#include "Timer.h"
#include "photosensors.h"
#include "firmware.h"

typedef struct {
    bool valid;
    bool skip;
} sync_pulse_t;

sync_pulse_t newPulse, oldPulse;

TimerClass timer(2);


// forward declarations (below order should help readability)
void armSyncPulse();
void measureSyncPulse();
void readSyncPulse(sync_pulse_t &pulse);
void armSweepPulse();
void measureSweepPulse();
void sendPulseData();


void pulseSetup() {
    photosensorSetup();

    // start timers, they will be sync'ed (reset) in PPI
    nrf_timer_task_trigger(nrf_timers[syncTimer], NRF_TIMER_TASK_START);
    nrf_timer_task_trigger(nrf_timers[forkTimer], NRF_TIMER_TASK_START);

    armSyncPulse();
}


void armSyncPulse() {
    PPI.resetChannels();

    // Clear old measures in case of missed signal:
    for (int i = 0; i < sensors_num; i++) {
        nrf_timer_cc_write(nrf_timers[syncTimer],
                nrf_timer_cc_channel_t(i), 0);
        nrf_timer_cc_write(nrf_timers[forkTimer],
                nrf_timer_cc_channel_t(i), 0);
    }

    PPI.setTimer(syncTimer);

    // sync timers using all photodiodes
    for (int i = 0; i < sensors_num; i++) {
        PPI.setInputPin(sensors_e[i]);

        // clear both timers using all photodiodes...
        PPI.setShortcut(PIN_LOW, TIMER_CLEAR, forkTimer);

        // ...and capture pulse widths on timer 1
        PPI.setShortcut(PIN_HIGH, TIMER_CAPTURE);
    }

    // wait for posedge
    while (digitalRead(sensors_e[0]) == 1);
    while (digitalRead(sensors_e[0]) == 0);
    measureSyncPulse();
}


void measureSyncPulse() {
    PPI.resetChannels();

    readSyncPulse(newPulse);

    if (newPulse.valid && !newPulse.skip) {
        // if old pulse is valid then the emitting base is B
        pulse_data.baseID = oldPulse.valid; // 0 = A  -  B = 1
        oldPulse.valid = false;
        armSweepPulse();
    } else {
        // otherwise try again
        oldPulse = newPulse;
        armSyncPulse();         // TODO implement a fail counter
    }
}


void readSyncPulse(sync_pulse_t &pulse) {
    for (int i = 0; i < sensors_num; i++) {
        pulse_data.captures[0][i] =
                nrf_timer_cc_read(nrf_timers[syncTimer],
                                  nrf_timer_cc_channel_t(i));

        // Look for at least 1 valid pulse                        TODO: make it smarter?
        int pulseWidthTicks16 = pulse_data.captures[0][i]; // 16 MHz

        if ( pulseWidthTicks16 > minSyncPulseWidth &&    // ticks
             pulseWidthTicks16 < maxSyncPulseWidth ) {   // ticks

            float pulseWidthTicks48 = pulseWidthTicks16 * 3; // convert to 48MHz ticks

            // for the following calculation, consult "Sync pulse timings" in .h file
            pulse_data.axis = (int(round(pulseWidthTicks48 / 500.)) % 2);

            pulse.skip = (pulse_data.captures[0][i] > skipThreshold); // 100us to ticks

            pulse.valid = true;
            break;
        }
    }
}


// Time stamp both rising and falling edges for the 4 photodiodes
// Timer S: sensor 0: channels: 0, 1 captures: 0, 1 (rising, falling edge)
//          sensor 1: channels: 2, 3 captures: 2, 3 (rising, falling edge)
// Timer F: sensor 2: channels: 0, 1 captures: 4, 5 (rising, falling edge)
//          sensor 3: channels: 2, 3 captures: 6, 7 (rising, falling edge)
void armSweepPulse() {
    PPI.resetChannels();

    for (int i = 0; i < sensors_num; i++) {
        PPI.setTimer(timerNumbers[i/2]);            // sync & fork timers

        PPI.setInputPin(sensors_e[i]);              // diode 0 to 3
        PPI.setShortcut(PIN_LOW, TIMER_CAPTURE);   // channel i*2
        PPI.setShortcut(PIN_HIGH,  TIMER_CAPTURE);   // channel i*2 + 1
    }

    // compute data collection time by removing the base offset
    int collectionTime = sweepEndTime;
    if (pulse_data.baseID == 1)
        collectionTime -= interSyncOffset; // microsec

    timer.attachInterrupt(&measureSweepPulse, collectionTime);
}


void measureSweepPulse() {
    // Timers S and F, on 4 channels (0 to 3)
    for (int t = 0; t < 2; t++) {
        for (int c = 0; c < sensors_num; c++) {
            pulse_data.captures[t][c] = nrf_timer_cc_read(nrf_timers[timerNumbers[t]],
                                                          nrf_timer_cc_channel_t(c));
        }
    }

    sendPulseData(); // TODO: take it out of this file!

    pulse_data.isReady = true;
    armSyncPulse(); // prepare for next loop
}


void sendPulseData() {
#if 0
    // Send readable data:

    Serial.print(pulse_data.baseID);
    Serial.print(pulse_data.axis);

    for (int t = 0; t < 2; t++) {
        for (int c = 0; c < 4; c += 2) {
            int sum = pulse_data.captures[t][c] + pulse_data.captures[t][c+1];
            Serial.print('\t');
            Serial.print(sum /= 2); // send centroid
        }
    }
    Serial.print('\n');
#else
    // Send binary data:

    Serial.write(0xFF); Serial.write(0xFF);         // send start headers

    uint8_t base_axis = pulse_data.baseID << 1  |  pulse_data.axis;
    Serial.write(base_axis);                        // send base ID and axis

    for (int t = 0; t < 2; t++) {                   // send captures
        for (int c = 0; c < sensors_num; c += 2) {

            // Invalid pulse by default
            int pulseStart = pulse_data.captures[t][c];
            int pulseEnd = pulse_data.captures[t][c+1];
            int pulseWidthTicks16 = pulseEnd - pulseStart;

            if ( pulseWidthTicks16 < minSweepPulseWidth ||     // ticks
                 pulseWidthTicks16 > maxSweepPulseWidth ) {    // ticks
                // mark the measures if they are invalid
                pulseStart = 0;
                pulseEnd = 0;
            }

            // remove 2 LSb (non-significant) to send data on 16 bits
            pulseStart >>= 2;
            pulseEnd >>= 2;

            Serial.write((pulseStart >> 0) & 0xFF);    // LSB first
            Serial.write((pulseStart >> 8) & 0xFF);    // MSB last

            Serial.write((pulseEnd >> 0) & 0xFF);    // LSB first
            Serial.write((pulseEnd >> 8) & 0xFF);    // MSB last
        }
    }
#endif
}


pulse_data_t pulse_data;

