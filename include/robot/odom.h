#ifndef ROBOT_ODOM_H
#define ROBOT_ODOM_H

#include "v5_apitypes.h"

// Data structure used for Points on 2D coordinate frames
typedef struct {
  double x;     // Horizontal coordinate [mm]
  double y;     // Vertical coordinate [mm]
  double theta; // Heading [deg]
} Pose2D_t;

void odomInit();
void odomPeriodic();

double odomFindDistance(Pose2D_t p_begin, Pose2D_t p_end);
double odomFindHeading(Pose2D_t p_begin, Pose2D_t p_end);

#endif // ROBOT_ODOM_H
