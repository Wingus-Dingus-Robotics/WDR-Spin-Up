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

using namespace vex;

// Global instances of VEX objects?
vex::brain Brain;

int commsThread() {
  this_thread::setPriority(thread::threadPriorityNormal);
  while (1) {
    commsUpdate(ALL);

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

  while (1) {
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
        break;
      case Autonomous:
        autoPeriodic();
        launcherPeriodic();
        turretPeriodic();
        break;
      case InitDriver:
        autoInterrupt();
        opcontrolInit();
        break;
      case Driver:
        opcontrolPeriodic();
        break;
    }

    // Yield for other threads
    this_thread::sleep_for(1);
  }
}
