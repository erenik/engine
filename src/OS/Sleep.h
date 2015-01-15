// Emil Hedemalm
// 2013-03-29
// updated 2013-06-13 by Fredrik Larsson

#ifndef AEONIC_SLEEP_H
#define AEONIC_SLEEP_H

#include "OS.h"

// For sleeping processes n stuff!
#if defined LINUX | defined OSX
#include <unistd.h>
//void SleepMs(int ms);
#define Sleep(ms) usleep(ms*1000)
#endif // LINUX

#ifdef WINDOWS
void Wait(int milliseconds);
//#define Sleep(ms) Sleep(ms)
#endif

/// MACCORENA
/*
#ifdef APPLE
void Sleep(float ms);
#endif
*/

#endif // AEONIC_SLEEP_H
