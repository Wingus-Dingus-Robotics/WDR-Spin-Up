#include "opcontrol.h"
#include "wdr.h"

// Launcher states
// static bool state_launcher_on = false;
// static bool state_launcher_short = false;
//TODO speed state (currently turns off when pushing low-range)

// Intake states
static bool state_match_load = false;

// Timers
static vex::timer timer_lifter = vex::timer();

void opcontrolInit() {
  // state_launcher_on = false;
  // state_launcher_short = false;
  state_match_load = false;
}

void opcontrolPeriodic() {
  controllerBtnStateUpdate();

  //
  // Drive
  //

  int32_t joyY = vexControllerGet(kControllerMaster, Axis3);
  int32_t joyX = vexControllerGet(kControllerMaster, Axis1);

  // Joystick deadzones
  if ( abs(joyY) < 20) {
    joyY = 0;
  }
  if ( abs(joyX) < 20) {
    joyX = 0;
  }

  // Split arcade
  driveSetPWMRamp(joyY + joyX, joyY - joyX);

  //
  // Intake
  //

  // Intake roller (and maybe turret roller?)
  if (controllerGetBtnState(kControllerMaster, ButtonR1)) {
    intakeSpin(-80);
    turretRollerSpinPWM(-127);
  }
  else if (controllerGetBtnState(kControllerMaster, ButtonR2)) {
    intakeSpin(80);
    turretRollerSpinPWM(127);
    intakeDeploy(true);
  }
  else if (state_match_load) {
    intakeSpin(-127);
  }
  else {
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
    launcherSetRPM(LAUNCHER_SPEED_HIGH.left_RPM, LAUNCHER_SPEED_HIGH.right_RPM);
  }
  if (controllerGetBtnState(kControllerMaster, ButtonRight)) {
    launcherSetRPM(0, 0);
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
  // Subsystem periodic functions (i.e. closed-loop controllers)
  //

  launcherPeriodic();
  turretPeriodic();
}