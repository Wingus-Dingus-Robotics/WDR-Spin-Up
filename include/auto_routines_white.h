#ifndef AUTO_ROUTINES_WHITE_H
#define AUTO_ROUTINES_WHITE_H

#include "v5_apitypes.h"
#include "robot/odom.h"

// Starting pose
Pose2D_t white_autoGetStartingPose(void);

// Autonomous routines (to be run as threads)
int white_autonomous1(void);
int white_autonomous2(void);
int white_autonomous3(void);
int white_autonomous4(void);
int white_autonomous5(void);
int white_autonomous6(void);
int white_autonomous7(void);
int white_autonomous8(void);
int white_autonomous9(void);
int white_autonomous10(void);

#endif // AUTO_ROUTINES_WHITE_H