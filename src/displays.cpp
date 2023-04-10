#include "displays.h"
#include "wdr.h"

using namespace vex;

/**
 * @brief Thread that controls which screen page is displayed on V5 Brain
 *
 */
int displayThread() {
  this_thread::setPriority(thread::threadPrioritylow);

  V5_TouchStatus status;
  vexTouchDataGet(&status);
  int32_t last_press_count = status.pressCount;
  uint8_t page_number = 1;
  while (1) {
    vexTouchDataGet(&status);
    if ((status.lastEvent == kTouchEventPress)
    && (last_press_count) != status.pressCount) {
      page_number++;
      if (page_number > 3)  page_number = 1;
    }
    last_press_count = status.pressCount;

    switch (page_number) {
      case 1:
      displayScreen_competition();
      break;

      case 2:
      displayScreen_currentLimit();
      break;

      default:
      // vexDisplayString(1, "Page %d doesn't exist", page_number);
      page_number = 1;
    }

    // Display update (double buffered)
    vexDisplayRender(true, true);   // Render back buffer to screen
    vexDisplayErase();              // Clear the new back buffer

    // Yield for a long time
    this_thread::sleep_for(100);
  }

  return 0;
}

/**
 * @brief Display competition relevant info
 * 
 */
void displayScreen_competition() {
  // First line: Competition status
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

  // Autonomous selector
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
}

/**
 * @brief Display V5 Smart motor current limit information.
 * 
 */
void displayScreen_currentLimit() {
  vexDisplayString(0, "Motor current limits");
}