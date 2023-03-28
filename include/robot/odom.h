#ifndef ROBOT_ODOM_H
#define ROBOT_ODOM_H

#include "v5_apitypes.h"

// TODO: What's needed for odometry?
// Different update functions depending on the type of movement.
// To update pose on global frame, must convert movement from robot frame (i.e. relative to robot)
// i.e. transform movement vector from robot frame to global frame

// Global coordinate system
#define CM_PER_TILE   61.0  // 2' tile ~= 2 * 30.5cm

// Odometry data types

// Data structure for global field position (used for global variable in wdr.h)

// Data structure used for Points on coordinate frames
// Used for Global Frame pose global variable in wdr.h
// Used for Robot Frame pose in drive functions
typedef struct {
  double x;     // in distance unit (e.g. tiles, cm)
  double y;
  double theta; // in [deg]?
} Pose2D_t;

// Data structure used for vectors?

// Odometry functions
static double ticksToDistance(int32_t encoder_ticks);
static double degreesToRadians(double degrees);
void odomInit();

void odomSetGlobalFrame(double x, double y, double theta);

void odomUpdateGlobalPosition(void);

#endif // ROBOT_ODOM_H
