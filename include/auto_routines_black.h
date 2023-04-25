#ifndef AUTO_ROUTINES_BLACK_H
#define AUTO_ROUTINES_BLACK_H

#include "v5_apitypes.h"
#include "robot/odom.h"

// Starting pose
Pose2D_t black_autoGetStartingPose(void);

// Autonomous routines (to be run as threads)
int black_autonomous1(void);
int black_autonomous2(void);
int black_autonomous3(void);
int black_autonomous4(void);
int black_autonomous5(void);
int black_autonomous6(void);
int black_autonomous7(void);
int black_autonomous8(void);
int black_autonomous9(void);
int black_autonomous10(void);

#endif // AUTO_ROUTINES_BLACK_H
