#ifndef PULSE_H
#define PULSE_H

#include <math.h>


typedef struct {
    int pulse_captures[4]    = {{0}};
    int sweep_captures[2][4] = {{0}}; // 2 timers, 4 channels
    bool baseID;
    bool axis; // 0 horizontal (x), 1 vertical (y) - TODO: check it!
    bool isReady;
} pulse_data_t;

const int syncTimer = 3;
const int forkTimer = 4;

const int timerNumbers[] = {syncTimer, forkTimer};

const int ticksPerMicrosec = 16; // because 16MHz

const int margin = 10;          // microsec
const int minSyncPulseWidth = (62.5 - margin) * ticksPerMicrosec;
const int maxSyncPulseWidth = (135  + margin) * ticksPerMicrosec;
const int skipThreshold = ((104 + ceil(93.8)) / 2) * ticksPerMicrosec;

const int interSyncOffset = 400; // microsec


    /* Sync pulse timings - a 10us margins is good practice
    j0	0	0	0	3000	62.5
    k0	0	0	1	3500	72.9
    j1	0	1	0	4000	83.3
    k1	0	1	1	4500	93.8
    j2	1	0	0	5000	104
    k2	1	0	1	5500	115
    j3	1	1	0	6000	125
    k3	1	1	1	6500	135
    */

const int minSweepPulseWidth = 1  * ticksPerMicrosec;
const int maxSweepPulseWidth = 35 * ticksPerMicrosec;

const int sweepStartTime = 1222 - margin; // microsec
const int sweepEndTime   = 6777 + margin; // microsec

const int sweepStartTicks = sweepStartTime * ticksPerMicrosec;
const int sweepEndTicks   = sweepEndTime   * ticksPerMicrosec;

typedef void (*funcPtr_t)();
void pulseSetup(funcPtr_t callback);

bool pulseDataIsReady();


extern pulse_data_t pulse_data;

#endif // PULSE_H
