#include "pulse.h"
#include "pinout.h"
#include "PPI.h"
#include "Timer.h"
#include <math.h>


//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print(x)
#define DEBUG_PRINTLN(x)   Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

typedef struct {
    bool valid;
    bool skip;
} sync_pulse_t;

sync_pulse_t newPulse, oldPulse;

TimerClass timer(2);

///////////////////////////////////////////////////////////
// TMP
const int sensors[] = {sensors_e[1], sensors_e[1],
                       sensors_e[0], sensors_e[0]};
///////////////////////////////////////////////////////////

// forward declarations (below order should help readability)
void armSyncPulse();
void measureSyncPulse();
void readSyncPulse(sync_pulse_t &pulse);
void armSweepPulse();
void measureSweepPulse();


void pulseSetup() {
    for (int i = 0; i < sensors_num; i++)   // TODO TS4231 setup
        pinMode(sensors[i], INPUT);         // TODO remove!

    // start timers, they will be sync'ed (reset) in PPI
    nrf_timer_task_trigger(nrf_timers[syncTimer], NRF_TIMER_TASK_START);
    nrf_timer_task_trigger(nrf_timers[forkTimer], NRF_TIMER_TASK_START);

    armSyncPulse();
}


void armSyncPulse() {
    PPI.resetChannels();
    for (int i=0; i<4; i++) {
        nrf_timer_cc_write(nrf_timers[syncTimer],
                nrf_timer_cc_channel_t(i), 0); // TODO: check
        nrf_timer_cc_write(nrf_timers[forkTimer],
                nrf_timer_cc_channel_t(i), 0); // TODO: check
    }

    PPI.setTimer(syncTimer);

    // sync timers using all photodiodes
    for (int i = 0; i < 4; i++) {
        PPI.setInputPin(sensors[i]);

        // clear both timers using all photodiodes...
        PPI.setShortcut(PIN_HIGH, TIMER_CLEAR, forkTimer);

        // ...and capture pulse widths on timer 1
        PPI.setShortcut(PIN_LOW, TIMER_CAPTURE);
    }

    // TODO: make it work and use constant [array] & margin constant?
    // +  explain timer stopped then started
    //  timer.attachInterrupt(&measureSyncPulse, 143); // microseconds

    // wait for negedge
    while (digitalRead(sensors[0]) == 0);
    while (digitalRead(sensors[0]) == 1);
    measureSyncPulse(); // TODO do it in loop
}


void measureSyncPulse() {
    PPI.resetChannels(); // TODO?

    readSyncPulse(newPulse);

    if (newPulse.valid && !newPulse.skip) {
        // if old pulse is valid then the emitting base is B
        pulse_data.baseID = oldPulse.valid; // 0 = A  -  B = 1
        oldPulse.valid = false;
        armSweepPulse();
    } else {
        // otherwise try again
        oldPulse = newPulse; // TODO: union?
        armSyncPulse();
    }
}


void readSyncPulse(sync_pulse_t &pulse) {
    for (int i = 0; i < 4; i++) {
        pulse_data.captures[0][i] = nrf_timer_cc_read(nrf_timers[syncTimer],
                                                      nrf_timer_cc_channel_t(i));
    }

    int ticks16 = pulse_data.captures[0][1];

    // TODO: analyse captures (average of 2 valid medians)
    if ( ticks16 < 150 * 16. &&    // us to ticks
         ticks16 > 60  * 16. ) {   // us to ticks

        pulse.valid = true;

        float ticks48 = ticks16 * 3; // convert to 48MHz ticks
        pulse_data.axis = (int(round(ticks48 / 500.)) % 2);

        pulse.skip = (pulse_data.captures[0][1] > 100*16); // 100us to ticks
    }
}


// Time stamp both rising and falling edges for the 4 photodiodes
// Timer S: sensor 0: channels: 0, 1 captures: 0, 1 (rising, falling edge)
//          sensor 1: channels: 2, 3 captures: 2, 3 (rising, falling edge)
// Timer F: sensor 2: channels: 0, 1 captures: 4, 5 (rising, falling edge)
//          sensor 3: channels: 2, 3 captures: 6, 7 (rising, falling edge)
void armSweepPulse() {
    PPI.resetChannels();

    for (int i = 0; i < 4; i++) {
        PPI.setTimer(timerNumbers[i/2]);            // sync & fork timers

        PPI.setInputPin(sensors[i]);                // diode 0 to 3
        PPI.setShortcut(PIN_HIGH, TIMER_CAPTURE);   // channel i*2
        PPI.setShortcut(PIN_LOW,  TIMER_CAPTURE);   // channel i*2 + 1
    }

    // TODO: use constant [array] & margin constant?
    // +  explain timer stopped then started
    timer.attachInterrupt(&measureSweepPulse, 7000); // microseconds
}


void measureSweepPulse() {
    // Timers S and F, on 4 channels (0 to 3)
    for (int t = 0; t < 2; t++) {
        for (int c = 0; c < 4; c++) {
            pulse_data.captures[t][c] = nrf_timer_cc_read(nrf_timers[timerNumbers[t]],
                                                          nrf_timer_cc_channel_t(c));
            pulse_data.captures[t][c] /= 16; // convert to microsec
        }
    }

#if 0
    // Send data:
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
    Serial.write(0xFF);
    Serial.write(0xFF);
    uint8_t base_axis = pulse_data.baseID << 1  |  pulse_data.axis;
    Serial.write(base_axis);
    for (int t = 0; t < 2; t++) {
        for (int c = 0; c < 4; c += 2) {
            int sum = pulse_data.captures[t][c] + pulse_data.captures[t][c+1];
            sum /= 2;                 // compute centroid
            Serial.write(sum & 0xFF); // invert endianness
            Serial.write(sum >> 8);
        }
    }
#endif

    pulse_data.isReady = true;
    armSyncPulse(); // prepare for next loop
}

pulse_data_t pulse_data;

