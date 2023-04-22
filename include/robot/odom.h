#ifndef ROBOT_ODOM_H
#define ROBOT_ODOM_H

#include "v5_apitypes.h"

// Data structure used for Points on 2D coordinate frames
typedef struct {
  double x;     // in distance unit (e.g. tiles, cm)
  double y;
  double theta; // in [deg]?
} Pose2D_t;

// Data structure used for vectors?

// Odometry functions
//setStartPose  // Where is robot starting on global frame?
//init    // Calculates offset of SBF frame from global frame, based on start pose
//double odomFindDistance(p_start, p_end)
//double odomFindAngle(p_start, p_end)
//


double degreesToRadians(double degrees);
void odomInit();

void odomSetGlobalFrame(double x, double y, double theta);

void odomUpdateGlobalPosition(void);

#endif // ROBOT_ODOM_H
