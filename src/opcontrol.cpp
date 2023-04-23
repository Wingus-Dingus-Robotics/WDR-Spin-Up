#include "opcontrol.h"
#include "wdr.h"

// Launcher states
// static bool state_launcher_on = false;
// static bool state_launcher_short = false;
//TODO speed state (currently turns off when pushing low-range)

// Intake states
static bool state_match_load = false;
static bool state_intake_stop = false;    // when intake full, and when turret loaded
static bool state_turret_loaded = false;
static bool state_roller_prev = false;    // make sure intake is disabled when releasing roller double btn

// Timers
static vex::timer timer_intake_full = vex::timer();
static vex::timer timer_lifter = vex::timer();

void opcontrolInit() {
  // state_launcher_on = false;
  // state_launcher_short = false;

  // Intake states
  state_match_load = false;
  state_intake_stop = false;
  state_turret_loaded = false;
  state_roller_prev = false;
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
  // Intake
  //

  if (intakeCountDiscs() == 3) {
    // Make sure discs are settled before loading
    if (timer_intake_full.time() > 150) {
      state_intake_stop = true;
      launcherFlickSetDiscs(3);
      // Automatically load turret
      intakeTurretLoadSequence();
    }
  } else {
    timer_intake_full.reset();

    // Allow intake to run if:
    // a) Haven't just collected 3 discs AND
    // b) Haven't still got discs in turret
    if (launcherFlickCountDiscs() == 0) {
      state_intake_stop = false;
    } else {
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
    intakeDeploy(false);
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
  if (controllerIsBtnPressed(kControllerMaster, ButtonX)) {
    if (state_match_load) {
      state_match_load = false;
      intakeMatchLoad(false);
      // intakeSpin(0);
    } else {
      state_match_load = true;
      intakeDeploy(true);
      // Should there be a timer delay here?
      intakeMatchLoad(true);
      // intakeSpin(-127);        // Turns on in the intake roller logic
    }
  }

  // Lift discs to turret (TODO: sequence)
  // Also brings intake deploy up (unless match loader is down)
  if (controllerGetBtnState(kControllerMaster, ButtonL1)) {
    // Start timer once tapped
    if (controllerIsBtnPressed(kControllerMaster, ButtonL1)) {
      launcherFlickSetDiscs(intakeCountDiscs());
      timer_lifter.reset();
    }
    // Lift discs to turret, intake deploy up
    intakeTurretLoad(true);
    if (!state_match_load)  intakeDeploy(false);
  } else {
    if (timer_lifter.time() > 500) {
      intakeTurretLoad(false);
    }
  }

  //
  // Launcher
  //

  // Flicker //TODO: Flick counter for auto
  if (controllerGetBtnState(kControllerMaster, ButtonL2)) {
    launcherFlickSequence(true);
  } else {
    launcherFlickSequence(false);
  }

  // Launcher presets
  if (controllerGetBtnState(kControllerMaster, ButtonDown)) {
    launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
  }
  if (controllerGetBtnState(kControllerMaster, ButtonLeft)) {
    launcherSetRPM(LAUNCHER_SPEED_MED.left_RPM, LAUNCHER_SPEED_MED.right_RPM);
  }
  if (controllerGetBtnState(kControllerMaster, ButtonUp)) {
    // launcherSetRPM(LAUNCHER_SPEED_HIGH.left_RPM, LAUNCHER_SPEED_HIGH.right_RPM);
    launcherSetRPM(0, 0);
  }
  if (controllerGetBtnState(kControllerMaster, ButtonRight)) {
    // launcherSetRPM(0, 0);
  }


  // // Launcher low
  // if (controllerIsBtnPressed(kControllerMaster, ButtonDown)) {
  //   if (!state_launcher_short) {
  //     // state_launcher_on = true;
  //     // launcherSetRPM(2500, 1700);
  //     launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
  //     state_launcher_short = true;
  //   } else {
  //     // state_launcher_on = false;
  //     launcherSetRPM(0, 0);
  //     state_launcher_short = false;
  //   }
  // }

  // // Launcher med
  // if (controllerIsBtnPressed(kControllerMaster, ButtonLeft)) {
  //   // launcherSetRPM(3200, 1700);
  //   launcherSetRPM(LAUNCHER_SPEED_MED.left_RPM, LAUNCHER_SPEED_MED.right_RPM);
  //   state_launcher_short = false;
  // }

  // // Launcher high
  // if (controllerIsBtnPressed(kControllerMaster, ButtonUp)) {
  //   // launcherSetRPM(3500, 2000);
  //   launcherSetRPM(LAUNCHER_SPEED_HIGH.left_RPM, LAUNCHER_SPEED_HIGH.right_RPM);
  //   state_launcher_short = false;
  // }

  //
  // Temp turret stuff (and special launcher speed)
  //

  if (controllerGetBtnState(kControllerMaster, ButtonA)) {
    turretSetAngle(100);
    // launcherSetRPM(2700, 1700);
    launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
    // state_launcher_short = true;
  } else if (controllerGetBtnState(kControllerMaster, ButtonB)) {
    turretSetAngle(-100);
    // launcherSetRPM(2700, 1700);
    launcherSetRPM(LAUNCHER_SPEED_WALL.left_RPM, LAUNCHER_SPEED_WALL.right_RPM);
    // state_launcher_short = true;
  } else {
    turretSetAngle(0);
  }

  // Hack to return to a non-special speed
  if (controllerIsBtnReleased(kControllerMaster, ButtonA) || controllerIsBtnReleased(kControllerMaster, ButtonA)) {
    // launcherSetRPM(2500, 1700);
    launcherSetRPM(LAUNCHER_SPEED_LOW.left_RPM, LAUNCHER_SPEED_LOW.right_RPM);
    // state_launcher_short = true;
  }

  //
  // End game string
  //

  if (controllerGetBtnState(kControllerMaster, ButtonRight) && controllerGetBtnState(kControllerMaster, ButtonX)) {
    miscString(true);
  }

  //
  // Subsystem periodic functions (e.g. for closed-loop controllers, dynamic current limiting)
  //

  intakePeriodic();
  launcherPeriodic();
  turretPeriodic();
}