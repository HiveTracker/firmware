#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#define DEBUG

#ifdef DEBUG
#include "Arduino.h"
#define DEBUG_PRINT(...)     Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)   Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

#endif // _FIRMWARE_H_
