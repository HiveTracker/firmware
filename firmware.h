#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#define DEBUG
#ifdef DEBUG
#include "Arduino.h"
#define DEBUG_PRINT(x)     Serial.print(x)
#define DEBUG_PRINTLN(x)   Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#endif // _FIRMWARE_H_

