#include "auto.h"
#include "wdr.h"
#include "auto_routines_white.h"
#include "auto_routines_black.h"

using namespace vex;

// Function pointer for selected autonomous routine
int (*auto_routine_ptr)();

// Thread to run selected autonomous routine
thread threadAuto;

// Autonomous state
bool auto_started = false;

// Opcontrol behaviours
int32_t auto_intake_pwm = 0;
bool auto_timetoload = false;

vex::timer auto_intake_full_timer = vex::timer();

/**
 * @brief Initialize autonomous mode
 * 
 * Selects autonomous routine to run based on potentiometer input
 */
void autoInit() {
  // Reset autonomous state(s)
  auto_started = false;

  // Reset opcontrol behaviour states
  auto_intake_pwm = 0;
  auto_timetoload = false;

  // Select autonomous routine
  uint32_t auto_select = miscGetAutoSelect();
  if (auto_select < 500) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous1 : &black_autonomous1;
  } else if (auto_select < 1000) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous2 : &black_autonomous2;
  } else if (auto_select < 1500) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous3 : &black_autonomous3;
  } else if (auto_select < 2000) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous4 : &black_autonomous4;
  } else if (auto_select < 2500) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous5 : &black_autonomous5;
  } else if (auto_select < 3000) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous6 : &black_autonomous6;
  } else if (auto_select < 3500) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous7 : &black_autonomous7;
  } else if (auto_select < 4000) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous8 : &black_autonomous8;
  } else if (auto_select < 4500) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous9 : &black_autonomous9;
  } else if (auto_select < 5000) {
    auto_routine_ptr = miscGetJumperID() ? &white_autonomous10 : &black_autonomous10;
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

  // Opcontrol behaviour -> auto Hack
  intakeSpin(auto_intake_pwm);

  if (auto_timetoload) {
    uint8_t n_discs_to_load = intakeCountDiscs();
    if ((launcherFlickCountDiscs() == 0) && (n_discs_to_load > 0)) {
      // Automatically load turret
      launcherFlickSetDiscs(n_discs_to_load);
      intakeTurretLoadSequence();
    }
    auto_timetoload = false;
  }

  // launcherPeriodic();
  // turretPeriodic();
  // intakePeriodic();
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
  return miscGetJumperID() ? white_autoGetStartingPose() : black_autoGetStartingPose();
}

//
// Autonomous routines. Remember to set starting pose above!
//

int autonomousNothing() {
  return 0;
}

// Autonomous routines moved to auto_routines_<turret-colour>.cpp