#ifndef AUTO_H
#define AUTO_H

#include "v5_apitypes.h"
#include "robot/odom.h"

// Choose which autonomous routine to run during initialization
void autoInit();

// Run selected routine
void autoPeriodic();

// Interrupt autonomous routine if competition mode switch occurs
void autoInterrupt();

// Used by odom.cpp
Pose2D_t autoGetStartingPose(void);

// Autonomous routines (to be run as threads)
// Have moved to auto_routines_<turret-colour> files
int autonomousNothing(void);

#endif // AUTO_H
