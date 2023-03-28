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

using namespace vex;

// Global instances of VEX objects?
vex::brain Brain;

int displayThread() {
  while (1) {
    switch (wdrGetCompStatus()) {
      case Disabled:
        vexDisplayCenteredString(0, "Disabled");
        break;
      case InitAutonomous:
        vexDisplayCenteredString(0, "Init Auto");
        break;
      case Autonomous:
        vexDisplayCenteredString(0, "Auto");
        break;
      case InitDriver:
        vexDisplayCenteredString(0, "Init Driver");
        break;
      case Driver:
        vexDisplayCenteredString(0, "Driver");
        break;
    }
    vexDisplayString(1, "Jumper ID: %d;    Auto_select: %d", miscGetJumperID(), miscGetAutoSelect());

    uint32_t auto_select = miscGetAutoSelect();
    if (auto_select < 500) {
      vexDisplayString(2, "1 drive tune");
    } else if (auto_select < 1000) {
      vexDisplayString(2, "2 launcher tune");
    } else if (auto_select < 1500) {
      vexDisplayString(2, "3 Left EZ roller preload (tile middle)");
    } else if (auto_select < 2000) {
      vexDisplayString(2, "4 Right EZ roller preload (tile top right)");
    } else if (auto_select < 2500) {
      vexDisplayString(2, "5 Programming skills");
    } else if (auto_select < 3000) {
      vexDisplayString(2, "6 autonomous 6");
    } else {
      vexDisplayString(2, "No auto selected");
    }
    

    // Update SBF data?
    vexDisplayString(4, "ENC1,2,3=%u,%u,%u", sbf_data.ENC1, sbf_data.ENC2, sbf_data.ENC3);
    // Brain.Screen.printAt(10, 100, "ENC1,2,3=%u,%u,%u                  ",
    //                         sbf_data.ENC1, sbf_data.ENC2, sbf_data.ENC3);

    vexDisplayString(6, "Heading: %f", driveGetHeading());
    vexDisplayString(7, "Distance: %f", driveGetDistance());

    // vexDisplayString(8, "Disc 1: %d", intakeGetDetectorReading(1));
    // vexDisplayString(9, "Disc 2: %d", intakeGetDetectorReading(2));
    // vexDisplayString(10, "Disc 3: %d", intakeGetDetectorReading(3));
    // vexDisplayString(11, "No. of discs: %d", intakeCountDiscs());

    vexDisplayString(8, "Launcher RPM L: %f", launcher_avg_RPM_L);
    vexDisplayString(9, "Launcher RPM R: %f", launcher_avg_RPM_R);
    vexDisplayString(10, "Turret angle: %f", turretGetAngle());
    vexDisplayString(11, "Turret value: %d", turretGetRawReading());

    // vexDisplayString(12, "Auto select: %d", miscGetAutoSelect());

    // Display update (double buffered)
    vexDisplayRender(true, true);   // Render back buffer to screen
    vexDisplayErase();              // Clear the new back buffer

    // Yield for a long time. 
    // TODO: Reduce priority of this thread too.
    this_thread::sleep_for(100);
  }

  return 0;
}

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
  threadDisplay.setPriority(vex::thread::threadPrioritylow);
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
