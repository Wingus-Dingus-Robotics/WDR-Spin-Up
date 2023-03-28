#ifndef AUTO_H
#define AUTO_H

#include "v5_apitypes.h"

// Choose which autonomous routine to run during initialization
void autoInit();

// Run selected routine
void autoPeriodic();

// Interrupt autonomous routine if competition mode switch occurs
void autoInterrupt();

// Autonomous routines (to be run as threads)
int autonomous1(void);
int autonomous2(void);
int autonomous3(void);
int autonomous4(void);
int autonomous5(void);
int autonomous6(void);
int autonomousNothing(void);

#endif // AUTO_H
