#include "opcontrol.h"
#include "wdr.h"

// Launcher states
// static bool state_launcher_on = false;
// static bool state_launcher_short = false;
//TODO speed state (currently turns off when pushing low-range)

bool fuck_off_aimbot = true;

V5Cmd_t LED_cmd = LED_ALIVE;
V5Cmd_t FLASH_cmd = FLASH_STOP;

// Intake states
static bool state_match_load = false;
static bool state_intake_stop = false;    // when intake full, and when turret loaded
static bool state_roller_prev = false;    // make sure intake is disabled when releasing roller double btn
static bool state_timetoload = false;    // when any launcher speed is set, it's time to shoot. Make sure discs are loaded.
static bool state_aimbot = false;   // turret tries to point towards goal, based on SBF pose data

// Timers
static vex::timer timer_intake_full = vex::timer();
static vex::timer timer_lifter = vex::timer();
static vex::timer timer_launcher_empty = vex::timer();
static vex::timer timer_string_sequence = vex::timer();

// Send to comms.cpp
V5Cmd_t opcontrolLED() {
  return LED_cmd;
}

V5Cmd_t opcontrolFlash() {
  return FLASH_cmd;
}

void opcontrolInit() {
  // state_launcher_on = false;
  // state_launcher_short = false;

  // Intake states
  state_match_load = false;
  state_intake_stop = false;
  state_roller_prev = false;
  state_aimbot = false;
}

void opcontrolPeriodic() {
  controllerBtnStateUpdate();

  //
  // Drive
  //

  int32_t joyY = vexControllerGet(kControllerMaster, Axis3);
  int32_t joyX = vexControllerGet(kControllerMaster, Axis1);

  // Joystick deadzones
  if ( abs(joyY) < 10) {
    joyY = 0;
  }
  if ( abs(joyX) < 10) {
    joyX = 0;
  }

  // Split arcade
  driveSetPWMRamp(joyY + joyX, joyY - joyX);

  //
  // LED
  //

  // No. of discs collected
  switch (intakeCountDiscs()) {
  case 1:
    LED_cmd = LED_GREEN;
    break;

  case 2:
    LED_cmd = LED_ORANGE;
    break;

  case 3:
    LED_cmd = LED_RED;
    break;

  default:
    if (launcherFlickCountDiscs() > 0) {
      // LED_cmd = LED_RED;
    } else {
      LED_cmd = LED_ALIVE;
    }
  }

  // Flashing
  if (launcherFlickCountDiscs()) {
    FLASH_cmd = FLASH_SLOW;
  } else {
    FLASH_cmd = FLASH_STOP;
  }

  // // TODO: Aimbot flashing
  // if (state_aimbot) {
  //   FLASH_cmd = FLASH_SLOW;
  // } else {
  //   FLASH_cmd = FLASH_STOP;
  // }

  //
  // Intake
  //

  if (intakeCountDiscs() == 3) {
    // Make sure discs are settled before loading
    if (timer_intake_full.time() > 150) {
      state_intake_stop = true;
      // Automatically load turret
      state_timetoload = true;
      // Default launcher speed: short-range
      launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
    }
  } else {
    timer_intake_full.reset();

    // Allow intake to run if:
    // a) Haven't just collected 3 discs AND
    // b) Haven't still got discs in turret
    if (launcherFlickCountDiscs() == 0) {
      if (timer_launcher_empty.time() > 500) {
        // Delay after shooting last disc before turning turret back to zero.
        state_intake_stop = false;
      }
    } else {
      timer_launcher_empty.reset();
      state_intake_stop = true; // e.g. manually loaded 1 disc in turret
    }
  }

  // Intake, intake deploy, turret roller
  if (controllerGetBtnState(kControllerMaster, ButtonR1) && 
  controllerGetBtnState(kControllerMaster, ButtonR2)) {
    // Turret roller
    intakeSpin(0);
    turretRollerSpinPWM(-127);
    intakeDeploy(false);
    state_roller_prev = true;
  } else if (state_roller_prev) {
    // Only allow intake to run again once both roller btns released
    intakeSpin(0);
    turretRollerSpinPWM(0);
    intakeDeploy(false);
    if (!controllerGetBtnState(kControllerMaster, ButtonR1) && 
    !controllerGetBtnState(kControllerMaster, ButtonR2)) {
      state_roller_prev = false;
    }
  } else if (controllerGetBtnState(kControllerMaster, ButtonR1)) {
    // Intake reverse
    intakeSpin(-127);   // Subsystem behaviour: won't spin until deployed
    turretRollerSpinPWM(0);
    intakeDeploy(true); // Intake can't spin while deploy up
  } else if (controllerGetBtnState(kControllerMaster, ButtonR2) && !(state_intake_stop)) {
    // Intake forward
    intakeSpin(127);
    turretRollerSpinPWM(0);
    intakeDeploy(true);
    // TODO: Check if intaking should be happening (e.g. not while shooting)
  } else if (state_intake_stop) {
    // Automatically bring intake up when full (also stop intake) OR when turret loaded
    // Note that intake reverse button can still bring intake down
    //TODO: More states when intake should be up
    if (!state_match_load) {
      intakeDeploy(false);
    }
    intakeSpin(0);
    turretRollerSpinPWM(0);
  } else if (state_match_load) {
    // Match loads
    intakeSpin(-127);
    turretRollerSpinPWM(0);
  } else {
    // Stop
    intakeSpin(0);
    turretRollerSpinPWM(0);
  }

  // Match loads
  if (controllerIsBtnPressed(kControllerMaster, ButtonY)) {
    if (state_match_load) {
      state_match_load = false;
      intakeMatchLoad(false);
      // Intake is turned off in the intake roller logic
    } else {
      state_match_load = true;
      intakeDeploy(true);
      // Intake is turned on in the intake roller logic
    }
  }

  if (state_match_load) {
    intakeMatchLoad(true);  // Deploy sequence will try to pull it back up...
  }

  // Lift discs to turret
  // Also brings intake deploy up (unless match loader is down)
  if (controllerGetBtnState(kControllerMaster, ButtonL1)) {
    // Start timer once tapped
    if (controllerIsBtnPressed(kControllerMaster, ButtonL1)) {
      launcherFlickSetDiscs(intakeCountDiscs());
      timer_lifter.reset();
    }
    // Lift discs to turret, intake deploy up
    intakeTurretLoad(true);
    // Default launcher speed: short-range
    if (!state_match_load) {
      launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
      intakeDeploy(false);
    }
  } else {
    if (timer_lifter.time() > 500) {
      intakeTurretLoad(false);
    }
  }

  //
  // Launcher
  //

  // Flicker
  if (controllerGetBtnState(kControllerMaster, ButtonL2)) {
    launcherFlickSequence(true);
  } else {
    launcherFlickSequence(false);
  }

  //
  // Turret and launcher
  //

  // TODO: Add a delay before launcher disc count decrements. 
  // When state_intake_stop is false, return turret to zero. Unless match loading

  if (controllerGetBtnState(kControllerMaster, ButtonUp)) {
    // Panic reset: Zero turret and launcher speed
    turretSetAngle(0);
    launcherSetRPM(0, 0);
    state_aimbot = false;
    state_match_load = false;
  } else if (controllerIsBtnPressed(kControllerMaster, ButtonDown)) {
    // Toggle
    state_match_load = false;
    if (state_aimbot) {
      state_aimbot = false;
      // Manual aiming (low speed, straight ahead)
      turretSetAngle(0);
      launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
      state_timetoload = true;
    } else {
      state_aimbot = true;
    }
  } else if (controllerGetBtnState(kControllerMaster, ButtonA)) {
    // Wall turret: Clockwise
    turretSetAngle(100);
    launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
    state_timetoload = true;
    state_aimbot = false;
    state_match_load = false;
  } else if (controllerGetBtnState(kControllerMaster, ButtonB)) {
    // Wall turret: Counter-clockwise
    turretSetAngle(-100);
    launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
    state_timetoload = true;
    state_aimbot = false;
    state_match_load = false;
  } else if (controllerIsBtnPressed(kControllerMaster, ButtonLeft) && state_intake_stop) {
    // Nudge turret Left (CCW)
    // Disabled until intake is stowed
    if (turretGetTargetAngle() >= -170) {
      turretSetAngle(turretGetTargetAngle() - 10);
    }
  } else if (controllerIsBtnPressed(kControllerMaster, ButtonRight) && state_intake_stop) {
    // Nudge turret right (CW)
    // Disabled until intake is stowed
    if (turretGetTargetAngle() <= 170) {
      turretSetAngle(turretGetTargetAngle() + 10);
    }
  } else if (state_match_load) {
    // Match load turret angle
    turretSetAngle(145);
  } else if (!state_intake_stop) {
    // If no discs left to launch, and intake pressed, return turret to zero
    // Generally when intaking, turret should be at zero.
    turretSetAngle(0);
  }

  if (state_timetoload) {
    uint8_t n_discs_to_load = intakeCountDiscs();
    if ((launcherFlickCountDiscs() == 0) && (n_discs_to_load > 0)) {
      // Automatically load turret
      launcherFlickSetDiscs(n_discs_to_load);
      intakeTurretLoadSequence();
    }
    state_timetoload = false;
  }

  if (state_aimbot && state_intake_stop && (!state_match_load)) {
    // Auto aiming
    // Issue: Can't turn on aimbot when robot thinks zero discs in turret
    // Issue: Lots of turret brake application. Either stay still, or turn off.
    // TODO: implement.
    double goal_x = 300;
    double goal_y = 300;
    double dx = goal_x - p_global.x;
    double dy = goal_y - p_global.y;
    double target_global_deg = -atan(dy/dx) * (180.0 / M_PI);
    double target_turret_deg = target_global_deg - (fmod(p_global.theta, 360));

    if ((target_turret_deg < -100)) target_turret_deg = -100;
    if ((target_turret_deg > 100)) target_turret_deg = 100;

    if (!fuck_off_aimbot)
      turretSetAngle(target_turret_deg);  // Note: Nudge should be made to still work during aimbot
    else
      turretSetAngle(0);
    
    launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
  }

  //
  // End game string
  //

  if (controllerGetBtnState(kControllerMaster, ButtonUp) && controllerGetBtnState(kControllerMaster, ButtonX)) {
    miscStringL(true);
    if (timer_string_sequence.time() > 200) {
      miscStringR(true);
    }
  } else {
    timer_string_sequence.reset();
  }

  //
  // Subsystem periodic functions (e.g. for closed-loop controllers, dynamic current limiting)
  //

  intakePeriodic();
  launcherPeriodic();
  turretPeriodic();
}