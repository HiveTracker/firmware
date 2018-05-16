/*
   PPI class for nRF52.
   Written by Chiara Ruggeri (chiara@arduino.org)

   Copyright (c) 2016 Arduino.  All right reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   Enjoy!
 */


#include "PPI.h"
#include "nRF_SDK/nrf_timer.h"

const nrf_ppi_channel_t channels[20] = {NRF_PPI_CHANNEL0, NRF_PPI_CHANNEL1,
    NRF_PPI_CHANNEL2, NRF_PPI_CHANNEL3, NRF_PPI_CHANNEL4, NRF_PPI_CHANNEL5,
    NRF_PPI_CHANNEL6, NRF_PPI_CHANNEL7, NRF_PPI_CHANNEL8, NRF_PPI_CHANNEL9,
    NRF_PPI_CHANNEL10, NRF_PPI_CHANNEL11, NRF_PPI_CHANNEL12, NRF_PPI_CHANNEL13,
    NRF_PPI_CHANNEL14, NRF_PPI_CHANNEL15, NRF_PPI_CHANNEL16, NRF_PPI_CHANNEL17,
    NRF_PPI_CHANNEL18, NRF_PPI_CHANNEL19};

const nrf_gpiote_tasks_t gpio_taskNo[] = {NRF_GPIOTE_TASKS_OUT_0,
    NRF_GPIOTE_TASKS_OUT_1, NRF_GPIOTE_TASKS_OUT_2, NRF_GPIOTE_TASKS_OUT_3,
    NRF_GPIOTE_TASKS_OUT_4, NRF_GPIOTE_TASKS_OUT_5, NRF_GPIOTE_TASKS_OUT_6,
    NRF_GPIOTE_TASKS_OUT_7};

const nrf_gpiote_events_t gpio_eventNo[] = {NRF_GPIOTE_EVENTS_IN_0,
    NRF_GPIOTE_EVENTS_IN_1, NRF_GPIOTE_EVENTS_IN_2, NRF_GPIOTE_EVENTS_IN_3,
    NRF_GPIOTE_EVENTS_IN_4, NRF_GPIOTE_EVENTS_IN_5, NRF_GPIOTE_EVENTS_IN_6,
    NRF_GPIOTE_EVENTS_IN_7};

NRF_TIMER_Type* const nrf_timers[] = { NRF_TIMER0, NRF_TIMER1, NRF_TIMER2,
                                   NRF_TIMER3, NRF_TIMER4 };

const nrf_gpiote_polarity_t gpio_polarity[] = { NRF_GPIOTE_POLARITY_LOTOHI,
    NRF_GPIOTE_POLARITY_HITOLO, NRF_GPIOTE_POLARITY_TOGGLE };

const nrf_timer_task_t capture_tasks[] = { NRF_TIMER_TASK_CAPTURE0,
    NRF_TIMER_TASK_CAPTURE1, NRF_TIMER_TASK_CAPTURE2, NRF_TIMER_TASK_CAPTURE3 };

//public functions

PPIClass::PPIClass() {
    timerNo = 1; // 0 is used by soft device
    configureTimer(3);
    configureTimer(4); // TODO CHEEEECK !
}


int PPIClass::setShortcut(event_type event, task_type task,
                          int forkTimer, task_type forkTask) {
    //check if there is still a free channel
    if(first_free_channel==20) {
        Serial.println("\n !!! WAAARNING !!! first_free_channel==20 !!!");
        return 0;
    }

    //enable sensing
    nrf_gpiote_event_enable(event_index);

    configureGPIOEvent(event);

    nrf_timer_task_t nrf_task = nrf_timer_task_t(task);


    static int capture_index = 0;
    if (task == TIMER_CAPTURE) {
        nrf_task = capture_tasks[capture_index];
        capture_index = (capture_index >= 3)? 0 : capture_index+1;
    }

    nrf_ppi_channel_endpoint_setup(channels[first_free_channel],
            (uint32_t)nrf_gpiote_event_addr_get(gpio_eventNo[event_index]),
            (uint32_t)nrf_timer_task_address_get(nrf_timers[timerNo], nrf_task));

    uint32_t fep = 0; // fork end point

    if (forkTimer >= 0 || forkTask != TIMER_NONE) {

        nrf_timer_task_t nrf_fork = (forkTask==TIMER_NONE)
                                    ? nrf_task
                                    : nrf_timer_task_t(forkTask);

        if (forkTimer < 0) {
            forkTimer = timerNo;
        }

        fep = (uint32_t)nrf_timer_task_address_get(nrf_timers[forkTimer], nrf_fork);
    }

    // always configure forks but if not necessary, use 0 as fork end point.
    nrf_ppi_fork_endpoint_setup(channels[first_free_channel], fep);

    nrf_ppi_channel_enable(channels[first_free_channel]);

    first_free_channel++;
    return (first_free_channel-1);
}


void PPIClass::setInputPin(uint32_t pin){
    inputPin=pin;
}

void PPIClass::setTimer(int _timerNo) {
    timerNo = _timerNo;
}


//private function

//functions to configure events
void PPIClass::configureTimer(int _timerNo){                                  // TODO: CHECK WITH INTERRUPT LIB!
    nrf_timer_mode_set(nrf_timers[_timerNo], NRF_TIMER_MODE_TIMER);
    nrf_timer_bit_width_set(nrf_timers[_timerNo], NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_frequency_set(nrf_timers[_timerNo], NRF_TIMER_FREQ_16MHz);
}

void PPIClass::configureGPIOEvent(event_type event){
    //if user use more than the allowed number of gpio, overwrite previous configuration
    if(gpiote_config_index==8) {
        Serial.println("\n !!! WAAARNING !!! gpiote_config_index==8 !!!");
        gpiote_config_index=0;
    }
    nrf_gpiote_event_configure(gpiote_config_index, inputPin, gpio_polarity[event]);

    event_index=gpiote_config_index;
    gpiote_config_index++;
}

void PPIClass::resetChannels(){
    nrf_ppi_channel_disable_all();
    first_free_channel = 0;
    gpiote_config_index = 0;
}


PPIClass PPI;

