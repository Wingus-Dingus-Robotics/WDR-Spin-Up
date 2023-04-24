#include "auto.h"
#include "wdr.h"

using namespace vex;

// Function pointer for selected autonomous routine
int (*auto_routine_ptr)();

// Thread to run selected autonomous routine
thread threadAuto;

// Autonomous state
bool auto_started = false;

/**
 * @brief Initialize autonomous mode
 * 
 * Selects autonomous routine to run based on potentiometer input
 */
void autoInit() {
  // Reset autonomous state(s)
  auto_started = false;

  // Select autonomous routine
  uint32_t auto_select = miscGetAutoSelect();
  if (auto_select < 500) {
    auto_routine_ptr = &autonomous1;
  } else if (auto_select < 1000) {
    auto_routine_ptr = &autonomous2;
  } else if (auto_select < 1500) {
    auto_routine_ptr = &autonomous3;
  } else if (auto_select < 2000) {
    auto_routine_ptr = &autonomous4;
  } else if (auto_select < 2500) {
    auto_routine_ptr = &autonomous5;
  } else if (auto_select < 3000) {
    auto_routine_ptr = &autonomous6;
  } else {
    auto_routine_ptr = &autonomousNothing;
  }
  
}

/**
 * @brief Run autonomous mode
 * 
 * Starts a new thread for selected autonomous routine.
 *
 * If autonomous mode ends before routine is finished, call autoInterrupt().
 */
void autoPeriodic() {
  // Start autonomous thread?
  if (!auto_started) {
    auto_started = true;
    threadAuto = thread(*auto_routine_ptr);
    // threadAuto.detach();
  }

  // Check if end of auto routine has been reached
  // if (threadAuto.joinable()) {
  //   threadAuto.join();
  // }

  //
  // Subsystem periodic functions (i.e. closed-loop controllers)
  //

  launcherPeriodic();
  turretPeriodic();
  // Note that drive closed-loop doesn't run in a periodic function.
  // Instead they're implemented as blocking movement functions.
  // Would probably be more flexible to implement controller in periodic, but whatever.
}

/**
 * @brief Stops the thread of the currently running autonomous routine
 * 
 */
void autoInterrupt() {
  if (auto_started) {
    threadAuto.interrupt();
    auto_started = false;
  }
}

Pose2D_t autoGetStartingPose() {
  Pose2D_t pose;
  uint32_t auto_select = miscGetAutoSelect();
  if (auto_select < 500) {
    // Autonomous 1
    pose = {0,0,0};
  } else if (auto_select < 1000) {
    // Autonomous 2
    pose = {0,0,0};
  } else if (auto_select < 1500) {
    // Autonomous 3
    pose = {0,0,0};
  } else if (auto_select < 2000) {
    // Autonomous 4
    pose = {0,0,0};
  } else if (auto_select < 2500) {
    // Autonomous 5
    pose = {0,0,0};
  } else if (auto_select < 3000) {
    // Autonomous 6
    pose = {0,0,0};
  } else {
    // Default
    pose = {0,0,0};
  }
  return pose;
}

//
// Autonomous routines. Remember to set starting pose above!
//

int autonomous1() {
  // driveMoveDistance(400, 40, 500);
  // wait(1000, msec);
  // driveMoveDistance(-400, -40, 500);
  // wait(1000, msec);

  // t_accel = max_v / max_a
  // x_accel = 0.5 * a * t_accel^2
  driveProfileDistance(1000, 30, 50, 1000);
  // wait(1000, msec);
  driveProfileDistance(-1000, -30, -50, 1000);
  wait(1000, msec);
  driveTurnAngle(90, 40, 500);
  wait(1000, msec);
  driveTurnAngle(-90, -40, 500);
  wait(1000, msec);

  return 0;
}

int autonomous2() {
  

  // Lock turret in position
  turretSetAngle(turretGetAngle());
  // intakeTurretLoad(true);

  // Shoot!

  // //Left
  // launcherSetRPM(4200, 1500);
  // // launcherSetRPM(2700, 2200);

  // Right
  // launcherSetRPM(3600, 2000);   // Saturday speeds
  launcherSetRPM(4400, 1500);

  wait(1500, msec);
  intakeTurretLoad(false);
  wait(500, msec);
  for (int i=0; i<2; i++) {
    launcherFlick(true);
    wait(1000, msec);
    launcherFlick(false);
    wait(1000, msec);
  }
  launcherSetRPM(0, 0);

  return 0;
}

int autonomous3() {
  // Setup:
  // - Start left side, in front of roller
  // - Point turret while setting up

  // Lock turret in position
  turretSetAngle(turretGetAngle());
  // intakeTurretLoad(true);

  // Wait for other robot to shoot first

  // Shoot!
  // launcherSetRPM(3500, 2000);   // Saturday speeds (50% success)
  launcherSetRPM(4200, 1500);
  wait(5000, msec);
  intakeTurretLoad(false);
  wait(500, msec);
  for (int i=0; i<2; i++) {
    launcherFlick(true);
    wait(1000, msec);
    launcherFlick(false);
    wait(1000, msec);
  }
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

int autonomous4() {
  // Setup
  // - Start right side, top right corner of tile, facing right
  // - Point turret while setting up

  // Lock turret in position
  turretSetAngle(turretGetAngle());
  // intakeTurretLoad(true);

  // Shoot!
  // launcherSetRPM(3600, 2000);   // Saturday speeds
  launcherSetRPM(4400, 1500);
  wait(1500, msec);
  intakeTurretLoad(false);
  wait(500, msec);
  for (int i=0; i<2; i++) {
    launcherFlick(true);
    wait(1000, msec);
    launcherFlick(false);
    wait(1000, msec);
  }
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

int autonomous5() {
  // Programming Skills
  // Setup
  // - In front of match loader, middle of tiles.
  // - Don't put preloads in until lifter is down (need to count them)

  //
  // Setup for loader
  //

  // Intake deploy, turn launcher on, turn turret to goal
  intakeDeploy(true);
  launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
  turretSetAngle(-95);
  wait(1000, msec);

  // Match loader down when ready
  intakeMatchLoad(true);
  intakeSpin(-127);

  //
  // Do 7 match loads (and 2 preloads)
  //

  // Load 1 disc (with 2 preloads)

  // Load 2 batches of 3 discs

  for (int i=0; i<3; i++) {
    // Wait until 3 discs collected
    timer timer_oops = timer();
    timer_oops.reset();
    while ((intakeCountDiscs() < 3) && (timer_oops.time() < 10000)) {
      wait(100, msec);
    }
    wait(500, msec);  // Wait for disk to settle
    intakeSpin(-10);

    // Score
    intakeTurretLoad(true);
    wait(500, msec);
    intakeTurretLoad(false);
    wait(500, msec);
    for (int i=0; i<3; i++) {
      launcherFlick(true);
      wait(500, msec);
      launcherFlick(false);
      wait(500, msec);
    }
    intakeSpin(-127);
  }

  // Stop match loading
  intakeSpin(0);
  launcherSetRPM(0, 0);
  intakeMatchLoad(false);
  turretSetAngle(0);

  wait(500, msec);
  intakeDeploy(false);

  //
  // Rollers
  //

  // Drive to 
  driveMoveDistance(-50, -40, 500);
  wait(500, msec);
  driveTurnAngle(-90, -40, 500);
  wait(500, msec);
  driveMoveDistance(-900, -40, 500);
  driveTurnAngle(-90, -40, 500);
  wait(500, msec);

  // Copy paste roller code

  // Drive backward into roller, spin a bit
  driveSetPWM(-30, -30);
  turretRollerSpinPWM(127);
  wait(500, msec);

  turretRollerSpinPWM(0);
  driveMoveDistance(200, 50, 500);

  // tiles points
  driveTurnAngle(-45, -40, 500);
  driveMoveDistance(350, 49, 500);

  return 0;
}

int autonomous6() {

  intakeDeploy(true);
  intakeSpin(127);
  driveMoveDistance(1600, 40, 500);
  driveTurnAngle(90, 40, 500);
  intakeSpin(0);
  driveSetPWM(40, 40);
  wait(2000, msec);
  driveSetPWM(20, 20);
  wait(1000, msec);
  driveSetPWM(0, 0);

  // Shoot 3 discs

  // Intake deploy, turn launcher on, turn turret to goal
  intakeDeploy(true);
  launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
  turretSetAngle(-95);
  wait(1000, msec);

  // Score
    intakeTurretLoad(true);
    wait(500, msec);
    intakeTurretLoad(false);
    wait(500, msec);
    for (int i=0; i<3; i++) {
      launcherFlick(true);
      wait(500, msec);
      launcherFlick(false);
      wait(500, msec);
    }
    intakeSpin(-127);

  // autonomous5();

  //
  // Setup for loader
  //

  

  // Match loader down when ready
  intakeMatchLoad(true);
  intakeSpin(-127);

  //
  // Do 7 match loads (and 2 preloads)
  //

  // Load 1 disc (with 2 preloads)

  // Load 2 batches of 3 discs

  for (int i=0; i<3; i++) {
    // Wait until 3 discs collected
    timer timer_oops = timer();
    timer_oops.reset();
    while ((intakeCountDiscs() < 3) && (timer_oops.time() < 10000)) {
      wait(100, msec);
    }
    wait(500, msec);  // Wait for disk to settle
    intakeSpin(-10);

    // Score
    intakeTurretLoad(true);
    wait(500, msec);
    intakeTurretLoad(false);
    wait(500, msec);
    for (int i=0; i<3; i++) {
      launcherFlick(true);
      wait(500, msec);
      launcherFlick(false);
      wait(500, msec);
    }
    intakeSpin(-127);
  }

  // Stop match loading
  intakeSpin(0);
  launcherSetRPM(0, 0);
  intakeMatchLoad(false);
  turretSetAngle(0);

  wait(500, msec);
  intakeDeploy(false);

  //
  // Rollers
  //

  // Drive to 
  driveMoveDistance(-50, -40, 500);
  wait(500, msec);
  driveTurnAngle(-90, -40, 500);
  wait(500, msec);
  driveMoveDistance(-900, -40, 500);
  driveTurnAngle(-90, -40, 500);
  wait(500, msec);

  // Copy paste roller code

  // Drive backward into roller, spin a bit
  driveSetPWM(-30, -30);
  turretRollerSpinPWM(127);
  wait(500, msec);

  turretRollerSpinPWM(0);
  driveMoveDistance(200, 50, 500);

  // tiles points
  driveTurnAngle(-45, -50, 500);
  driveMoveDistance(350, 80, 500);

  return 0;
}

int autonomousNothing() {
  return 0;
}