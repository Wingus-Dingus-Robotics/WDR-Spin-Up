/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       shaun                                                     */
/*    Created:      2/4/2023, 10:46:37 AM                                     */
/*    Description:  V5 project (this is still true)                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include "wdr.h"
#include "opcontrol.h"
#include "auto.h"
#include "displays.h"

#include <array>

using namespace vex;

// Main loop timing globals
uint64_t main_execution_time_us, main_yield_time_us;

int commsThread() {
  this_thread::setPriority(thread::threadPriorityNormal);
  while (1) {
    commsUpdate(POSE);
    odomPeriodic();

    this_thread::sleep_for(10);
  }
  return 0;
}


int main() {
  // Run robot subsystem initialization functions
  commsInit();

  thread threadComms = thread(commsThread);
  threadComms.detach();

  // Run remaining subsystem init functions
  driveInit();
  intakeInit();
  launcherInit();
  miscInit();
  turretInit();

  thread threadDisplay = thread(displayThread);
  threadDisplay.detach();

  // Loop timing
  wdr_highres_timer_t main_loop_timer;
  uint64_t t_execution_us, t_yield_us;
  uint8_t loop_counter = 0;
  std::array<uint64_t, 10> array_t_execution, array_t_yield;
  array_t_execution.fill(0);
  array_t_yield.fill(0);

  // Odometry init
  bool first_comp_enable = false;

  while (1) {
    wdrHighResTimerReset(&main_loop_timer);
    wdrHighResTimerStart(&main_loop_timer);

    // Run correct competition mode
    wdrUpdateCompStatus();
    switch (wdrGetCompStatus()) {
      case Disabled:
        // TODO: Run disabled methods etc.
        autoInterrupt();
        driveDisable();
        intakeDisable();
        launcherDisable();
        turretDisable();
        break;
      case InitAutonomous:
        autoInit();
        if (!first_comp_enable) {
          first_comp_enable = true;
          odomInit();
        }
        break;
      case Autonomous:
        autoPeriodic();
        intakePeriodic();
        launcherPeriodic();
        turretPeriodic();
        break;
      case InitDriver:
        autoInterrupt();
        opcontrolInit();
        if (!first_comp_enable) {
          first_comp_enable = true;
          odomInit();
        }
        break;
      case Driver:
        opcontrolPeriodic();
        break;
    }

    // Yield and timing
    t_execution_us = wdrHighResTimerGetTime(&main_loop_timer);
    this_thread::sleep_for(1);  // Yield for other threads
    t_yield_us = wdrHighResTimerGetTime(&main_loop_timer) - t_execution_us;

    // Calculations for average loop timing
    if (loop_counter < 10) {
      array_t_execution[loop_counter] = t_execution_us;
      array_t_yield[loop_counter] = t_yield_us;
      loop_counter++;
    } else {
      loop_counter = 0;

      uint64_t sum_exec = 0;
      uint64_t sum_yield = 0;
      for (uint8_t i = 0; i < array_t_execution.size(); i++) {
        sum_exec += array_t_execution[i];
        sum_yield += array_t_yield[i];
      }
      main_execution_time_us = sum_exec / array_t_execution.size();
      main_yield_time_us = sum_yield / array_t_yield.size();
    }
  }
}
