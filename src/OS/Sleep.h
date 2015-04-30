// Emil Hedemalm
// 2013-03-29
// updated 2013-06-13 by Fredrik Larsson

#ifndef AEONIC_SLEEP_H
#define AEONIC_SLEEP_H

/// Required on Win32 to get better timed sleeps.
void SetTimerResolution(int milliseconds);
/// Resets to default value. Call on exit.
void ResetTimerResolution(); 

void SleepThread(int milliseconds);

#endif // AEONIC_SLEEP_H
