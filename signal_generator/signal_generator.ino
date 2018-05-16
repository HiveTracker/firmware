#if 0
const int testPins[] = {0, 1, 2, 3};

/* approximative timings in microseconds
    _                __
___| |_____...______|  |_____...
   0 100         4000  4200

*/

void setup()
{
    for (int i = 0; i < 4; i++)
        pinMode(testPins[i], OUTPUT);
}


void loop()
{
    for (int i = 0; i < 4; i++) {
        digitalWriteFast(testPins[i], HIGH);    // FAST digital write
        delayMicroseconds(500);
    }

    delay(1);

    for (int i = 0; i < 4; i++) {
        digitalWriteFast(testPins[i], LOW);
        delayMicroseconds(500);
    }

    delay(30);
}

#else

// TODO later:

/* Timings examples in microseconds
    ______              ______                       _                       ___
___|      |____________|      |_________..._________| |_________..._________|
   0      62           400    462                5000 5008                  8333
*/

const int maxPin = 4;
int posTimings[] = {0, 400, 5000};
int negTimingsJ0[] = {63, 504, 5008};
int negTimingsK0[] = {73, 515, 5008};
int negTimingsJ1[] = {83, 525, 5008};
int negTimingsK1[] = {94, 535, 5008};
int negTimingsJ2[] = {104, 463, 5008};
int negTimingsK2[] = {115, 473, 5008};
int negTimingsJ3[] = {125, 483, 5008};
int negTimingsK3[] = {135, 494, 5008};
const int loopTime = 8333;

const int timingsNum = sizeof posTimings / sizeof posTimings[0];


void setup()
{
    for (int k = 0; k < maxPin; k++)
      pinMode(k, OUTPUT);
    Serial.begin(115200);
    Serial.println("Starting...");
}

void runCycle(int negTimings[])
{
    // start first loop
    int t0 = micros();

    // this redundant positive edge improves the "zero" timing reference
    for (int k = 0; k < maxPin; k++)
      digitalWrite(k, HIGH);

    for (int i = 0; i < timingsNum; i++)
    {
        while (micros() <= posTimings[i] + t0);         // wait for pos edge
        for (int k = 0; k < maxPin; k++)
          digitalWrite(k, HIGH);                    // positive edge

        while (micros() <= negTimings[i] + t0);         // wait for neg edge
        for (int k = 0; k <  maxPin; k++)
          digitalWrite(k, LOW);                     // negative edge
    }

    while (micros() < loopTime + t0)                    // wait for next loop
    {
    }
}

void loop()
{
    runCycle(negTimingsJ0);
    runCycle(negTimingsK0);
    runCycle(negTimingsJ1);
    runCycle(negTimingsK1);
    runCycle(negTimingsJ2);
    runCycle(negTimingsK2);
    runCycle(negTimingsJ3);
    runCycle(negTimingsK3);
}
#endif
