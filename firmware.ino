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

void loop() {

//  nrf_timer_task_trigger(nrf_timers[syncTimer], NRF_TIMER_TASK_CAPTURE4);
//  int capture = nrf_timer_cc_read(nrf_timers[syncTimer],
//                                  nrf_timer_cc_channel_t(4));

//  Serial.print(capture);
//  Serial.print(" ");

//  if (pulse_data.isReady) {
//      for (int t = 0; t < 2; t++) {
//          for (int c = 0; c < 4; c++) {
//              // convert to microseconds
//              Serial.println(pulse_data.captures[t][c]/16.);
//          }
//      }
//      Serial.println();
//      pulse_data.isReady = false;
//  }
    // collect & send data
}


