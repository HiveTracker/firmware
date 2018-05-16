// This file adapts the pinout for the HiveTracker V1.0

#ifndef _PINOUT_
#define _PINOUT_

// LEDs
#define LED_R                (10)
#define LED_G                (14)
#define LED_B                (12)
#undef  PIN_LED
#define PIN_LED              LED_G
#define LED_BUILTIN          PIN_LED
#define BUILTIN_LED          PIN_LED

// Button
#define BUTTON               (26)

// Serial
#undef PIN_SERIAL_RX
#undef PIN_SERIAL_TX
#define PIN_SERIAL_RX        (25) /* J4 - E */
#define PIN_SERIAL_TX        (27) /* J4 - D */

// I2C
#undef PIN_WIRE_SDA
#undef PIN_WIRE_SCL
#define PIN_WIRE_SDA         (29)
#define PIN_WIRE_SCL         (28)

// Photodiodes
const int sensors_num = 4; // 0,  1,  2,  3
const int sensors_e[] =     {22, 18, 20, 19}; // Envelope line
const int sensors_d[] =     {24, 21,  4, 11}; // Data line - not used for now

#endif // _PINOUT_

