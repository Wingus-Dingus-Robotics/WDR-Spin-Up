#include "auto_routines_white.h"
#include "wdr.h"

using namespace vex;

static double A_ANGLE = 30;
static double V_ANGLE = 30;

Pose2D_t white_autoGetStartingPose(void) {
  Pose2D_t pose;
  uint32_t auto_select = miscGetAutoSelect();
  if (auto_select < 500) {
    // Autonomous 1
    pose = {1800,1800,0};   // Middle of field
  } else if (auto_select < 1000) {
    // Autonomous 2
    pose = {1800,1800,0};   // Middle of field
  } else if (auto_select < 1500) {
    // Autonomous 3
    pose = {1000,400,0};    // Back wall, roller front of roller, top right of tile
  } else if (auto_select < 2000) {
    // Autonomous 4
    pose = {3200,2200,0};   // Right wall, top left of tile, facing front wall
  } else if (auto_select < 2500) {
    // Autonomous 5
    pose = {1000,400,0};    // Same as auto 3
  } else if (auto_select < 3000) {
    // Autonomous 6
    pose = {3300,2100,270};   // Right wall, middle of tile, facing disc (theta=270)
  } else if (auto_select < 3500) {
    // Autonomous 7
    pose = {0,0,0};
  } else if (auto_select < 4000) {
    // Autonomous 8
    pose = {0,0,0};
  } else {
    // Default
    pose = {0,0,0};
  }
  return pose;
}

/**
 * @brief Drive tuning
 * 
 * Power/velocity values from here are probably okay to use.
 */
int white_autonomous1() {
  
  
  driveProfileAngle(-90, -A_ANGLE, -V_ANGLE, 1000);
  wait(500,msec);
  driveProfileAngle(90, A_ANGLE, V_ANGLE, 1000);

  // // t_accel = max_v / max_a
  // // x_accel = 0.5 * a * t_accel^2
  // driveProfileDistance(1000, 30, 50, 1000);
  // // wait(1000, msec);
  // driveProfileDistance(-1000, -30, -50, 1000);
  // wait(1000, msec);
  // driveTurnAngle(90, 40, 500);
  // wait(1000, msec);
  // driveTurnAngle(-90, -40, 500);
  // wait(1000, msec);

  return 0;
}

/**
 * @brief Launcher tuning
 * 
 * Use as reference for launch sequences
 */
int white_autonomous2() {
  // Lock turret in position
  turretSetAngle(turretGetAngle());

  // Intake until 3 discs collected, or 10 sec
  intakeDeploy(true);
  wait(500, msec);
  auto_intake_pwm = 127;
  while (intakeCountDiscs() < 3) {
    wait(50, msec);
  }
  wait(1000, msec);
  auto_intake_pwm = 0;


  // Load
  intakeTurretLoadSequence();
  // launcherSetRPM(4200, 1500);
  launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
  wait(2000, msec);

  // Shoot discs
  launcherFlickSetDiscs(3);
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(50, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);
  launcherSetRPM(0, 0);

  return 0;
}

/**
 * @brief Left: EZ roller
 *
 * Start in front of roller.
 * 
 * Shoot 2 preloads, spin roller.
 */
int white_autonomous3() {
  // Setup:
  // - Start left side, in front of roller
  // - Point turret while setting up

  // Lock turret in position
  turretSetAngle(turretGetAngle());

  // Wait for other robot to shoot first
  launcherSetRPM(3700-400, 2450-400);
  wait(7000, msec);
  intakeTurretLoad(false);
  wait(500, msec);

  // Shoot discs
  launcherFlickSetDiscs(2);
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);
  launcherSetRPM(0, 0);

  // Lock turret at zero
  turretSetAngle(0);
  wait(1000, msec);

  // Drive backward into roller, spin a bit
  driveSetPWM(-30, -30);
  turretRollerSpinPWM(127);
  wait(500, msec);

  turretRollerSpinPWM(0);
  driveMoveDistance(200, 50, 500);

  return 0;
}

/**
 * @brief Right EZ roller
 * 
 * @return int 
 */
int white_autonomous4() {
  // Setup
  // - Start right side, top right corner of tile, facing right
  // - Point turret while setting up

  // Lock turret in position
  turretSetAngle(turretGetAngle());
  // intakeTurretLoad(true);

  // Turn on launcher
  launcherSetRPM(3700, 2700);
  wait(1500, msec);

  // Shoot discs
  launcherFlickSetDiscs(2);
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);
  launcherSetRPM(0, 0);

  // Lock turret at zero
  turretSetAngle(0);
  wait(1000, msec);

  // Drive to roller
  driveMoveDistance(500, 40, 500);
  wait(500, msec);
  driveTurnAngle(-90, -40, 500);

  // Drive backward into roller, spin a bit
  driveSetPWM(-30, -30);
  turretRollerSpinPWM(127);
  wait(750, msec);

  turretRollerSpinPWM(0);
  driveMoveDistance(200, 50, 500);

  return 0;
}

/**
 * @brief Left safe route
 * 
 */
int white_autonomous5() {
  // Reuse Left EZ roller auto
  white_autonomous3();
  // driveTurnAngle(45, 40, 1000);
  driveProfileAngle(45, A_ANGLE, V_ANGLE, 1000);

  // Intake stack of 3, load
  intakeDeploy(true);
  wait(500, msec);
  auto_intake_pwm = 127;
  driveProfileDistance(1000, 30, 30, 1000);
  wait(500, msec);
  auto_intake_pwm = 0;
  auto_timetoload = true;
  intakeDeploy(false);
  wait(1000, msec);

  // Aim turret
  launcherSetRPM(3400-200, 1700-200);
  turretSetAngle(-75);
  wait(2000, msec);

  // Shoot discs
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);
  // launcherSetRPM(0, 0);

  // Reset turret, drive to discs
  turretSetAngle(0);
  // driveTurnAngle(-45, 40, 1000);
  driveProfileAngle(-45, -A_ANGLE, -V_ANGLE, 1000);
  wait(500, msec);
  driveProfileDistance(-700, -30, -70, 1000);
  wait(500, msec);
  // driveTurnAngle(90, 40, 1000);
  driveProfileAngle(90, A_ANGLE, V_ANGLE, 1000);
  wait(500, msec);
  driveProfileDistance(480, 30, 70, 1000);
  wait(500, msec);
  // driveTurnAngle(-90, 40, 1000);
  driveProfileAngle(-90, -A_ANGLE, -V_ANGLE, 1000);

  // Deploy intake
  intakeDeploy(true);
  wait(500, msec);

  // Intake 3 discs, load
  auto_intake_pwm = 127;
  driveProfileDistance(600, 30, 50, 1000);
  wait(500, msec);
  auto_intake_pwm = 0;
  auto_timetoload = true;
  intakeDeploy(false);
  // wait(1000, msec);

  // Aim turret
  launcherSetRPM(3600-200, 2000-200);
  turretSetAngle(-35);
  wait(2000, msec);

  // Shoot discs
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);
  launcherSetRPM(0, 0);

  return 0;
}

/**
 * @brief Right safe route
 * 
 */
int white_autonomous6() {
  // Setup
  // - In front of first disc. Middle of tile.

  /////// Point and shoot

  // Lock turret in position
  turretSetAngle(turretGetAngle());

  // Shoot first.
  launcherSetRPM(3700-400, 2700-400);
  wait(1000, msec);
  intakeTurretLoad(false);
  wait(500, msec);

  // Shoot discs
  launcherFlickSetDiscs(2);
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);

  // Lock turret at zero
  turretSetAngle(0);
  // wait(1000, msec);

  ///////////////////

  // Deploy intake, collect disc 1
  intakeDeploy(true);
  wait(500, msec);
  auto_intake_pwm = 127;
  driveProfileDistance(600, 30, 50, 1000);
  wait(500, msec);

  // Turn to discs, collect remaining discs, load
  // driveTurnAngle(-45, 40, 1000);
  driveProfileAngle(-45, -A_ANGLE, -V_ANGLE, 1000);
  wait(500, msec);
  driveProfileDistance(900, 30, 50, 1000);
  wait(500, msec);
  auto_intake_pwm = 0;
  auto_timetoload = true;
  intakeDeploy(false);
  // wait(1000, msec);

  // Aim turret
  launcherSetRPM(3400-100, 1700-100);
  turretSetAngle(90);
  wait(2000, msec);

  // Shoot discs (2nd)
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);

  //////////////////////

  // Reset turret, turn to discs
  turretSetAngle(0);
  // driveTurnAngle(-135, 40, 1000);
  driveProfileAngle(-135, -A_ANGLE, -V_ANGLE, 1000);
  
  // Deploy intake, collect 3 discs, load (no turret yet)
  intakeDeploy(true);
  wait(500, msec);
  auto_intake_pwm = 127;
  driveProfileDistance(1200, 30, 50, 1000);
  wait(500, msec);
  auto_intake_pwm = 0;
  intakeDeploy(false);
  auto_timetoload = true;

  // Drive to roller
  // driveTurnAngle(-90, 40, 1000);
  driveProfileAngle(-90, -A_ANGLE, -V_ANGLE, 1000);
  wait(500, msec);
  driveProfileDistance(1250, 50, 70, 1000);
  wait(500, msec);

  // Aim turret
  launcherSetRPM(3700-300, 2300-300);
  turretSetAngle(-85);
  wait(2000, msec);

  // Shoot 3 discs
  while (launcherFlickCountDiscs() > 0) {
    launcherFlickSequence(true);
    wait(500, msec);
  }
  wait(500, msec);
  launcherFlickSequence(false);

  // Reset turret, turn to roller
  launcherSetRPM(0, 0);
  turretSetAngle(0);
  // driveTurnAngle(-90, 40, 1000);
  driveProfileAngle(-90, -A_ANGLE, -V_ANGLE, 1000);
  wait(500, msec);

  // Drive backward into roller, spin a bit
  driveSetPWM(-30, -30);
  wait(1000, msec);
  turretRollerSpinPWM(127);
  wait(300, msec);

  turretRollerSpinPWM(0);
  driveMoveDistance(150, 50, 500);

  return 0;
}

/**
 * @brief Reserved for F-off left
 * 
 */
int white_autonomous7() {
  return 0;
}

/**
 * @brief Reserved for F-off right
 * 
 */
int white_autonomous8() {
  return 0;
}

int white_autonomous9() {
  return 0;
}

int white_autonomous10() {
  return 0;
}