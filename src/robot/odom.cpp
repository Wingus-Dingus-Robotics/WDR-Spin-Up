#include "robot/odom.h"
#include "wdr.h"
#include "auto.h"

// Global variables
Pose2D_t p_sbf, p_global, p_start;

// In SBF frame, SBF pose is offset from zero (because sending unsigned int)
// Default offset on bootup
static int32_t offset_x = 10000000; // 1000m --> [tenths of mm]
static int32_t offset_y = 10000000; // 1000m --> [tenths of mm]
static int32_t offset_theta = 1000 * 3600;  // 1000 rotations --> [tenths of deg]

/**
 * @brief Initializes SBF pose offset
 * 
 * Should be called once, the first time robot is in an enabled competition mode.
 */
void odomInit() {
  // Find how far away SBF pose is from {0,0,0} in the SBF frame
  // Make it the new offset. Remember uint -> int conversion!
  offset_x = (int32_t)sbf_pose.x;
  offset_y = (int32_t)sbf_pose.y;
  offset_theta = (int32_t)sbf_pose.theta;

  // Now that offset has been adjusted, sbf - offset = 0
  // Starting pose in SBF frame is now {0,0,0}

  // Starting pose in Global frame is determined by autoGetStartingPose()
}

/**
 * @brief Updates robot pose
 * 
 * Called in commsThread
 */
void odomPeriodic() {
  /* Set origin pose with auto selector */
  p_start = autoGetStartingPose();

  /* Update current robot pose */
  // {0,0,0} on SBF frame is p_start on Global frame

  // Get robot pose in SBF frame
  // Remember conversions (uint -> int) (tenths -> unit)
  p_sbf.x = ((int32_t)sbf_pose.x - offset_x) / 10.0;
  p_sbf.y = ((int32_t)sbf_pose.y - offset_y) / 10.0;
  p_sbf.theta = ((int32_t)sbf_pose.theta - offset_theta) / 10.0;

  // Transform pose from SBF frame to Global frame
  p_global.theta = p_start.theta + p_sbf.theta;
  double theta_rad = p_start.theta * (M_PI / 180.0);
  // Yes, theta_rad is a constant angle. (angle of SBF frame to Global frame. NOT robot angle.)
  p_global.x = p_start.x + ( p_sbf.x * cos(theta_rad) + p_sbf.y * sin(theta_rad) );
  p_global.y = p_start.y + ( p_sbf.x * -sin(theta_rad) + p_sbf.y * cos(theta_rad) ); 
}
