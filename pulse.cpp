#include <math.h>
#include "pulse.h"
#include "PPI.h"
#include "Timer.h"
#include "photosensors.h"
#include "firmware.h"
#include "pinout.h" // important to keep it last for the undefs to work

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
bool allAreHigh();



static void defaultFunc() {};
static funcPtr_t dataTXcallback;

void pulseSetup(funcPtr_t callback) {

    dataTXcallback = (callback) ? callback : defaultFunc;

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

    // Active wait till the end of sync pulses of multiple photodiodes
    while (allAreHigh());  // wait till we get low
    while (!allAreHigh()); // wait till we get high

    measureSyncPulse();
}


bool allAreHigh() {
    // TODO: timer or GPIO interrupt should work with the right priority:
    // https://github.com/HiveTracker/firmware/blob/2afe9f1/Timer.cpp#L25-L30
    // https://github.com/HiveTracker/PPI/commit/d3a54ad

    // photodiodes are high by default (low when light is detected)
    bool allHigh = digitalRead(sensors_e[0]) & digitalRead(sensors_e[1]) &
                   digitalRead(sensors_e[2]) & digitalRead(sensors_e[3]);
    return allHigh;
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
        pulse_data.pulse_captures[0][i] =
                nrf_timer_cc_read(nrf_timers[syncTimer],
                                  nrf_timer_cc_channel_t(i));

        // Look for at least 1 valid pulse                        TODO: make it smarter!!!
        int pulseWidthTicks16 = pulse_data.pulse_captures[0][i]; // 16 MHz

        if ( pulseWidthTicks16 > minSyncPulseWidth &&    // ticks
             pulseWidthTicks16 < maxSyncPulseWidth ) {   // ticks

            float pulseWidthTicks48 = pulseWidthTicks16 * 3; // convert to 48MHz ticks

            // for the following calculation, consult "Sync pulse timings" in .h file
            pulse_data.axis = (int(round(pulseWidthTicks48 / 500.)) % 2);

            pulse.skip = (pulse_data.pulse_captures[0][i] > skipThreshold); // 100us to ticks

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
            pulse_data.sweep_captures[t][c] = nrf_timer_cc_read(nrf_timers[timerNumbers[t]],
                                                          nrf_timer_cc_channel_t(c));
        }
    }

    // If CPU is sleeping most of the time, then it's probably OK to call it from an interrupt:
    // dataTXcallback();
    // otherwise, let's poll the follwing flag in the main loop:
    pulse_data.isReady = true;
    armSyncPulse(); // prepare for next loop
}


bool pulseDataIsReady() {
    if (pulse_data.isReady) {
        pulse_data.isReady = false;
        return true;
    } else {
        return false;
    }
}


pulse_data_t pulse_data;

