#include "displays.h"
#include "wdr.h"

using namespace vex;

/**
 * @brief Thread that controls which screen page is displayed on V5 Brain
 *
 */
int displayThread() {
  this_thread::setPriority(thread::threadPrioritylow);

  // TODO: Create an array of function pointers to different screens to display
  // instead of manually adding a new case and incrementing the max page_number

  V5_TouchStatus status;
  vexTouchDataGet(&status);
  int32_t last_press_count = status.pressCount;
  uint8_t page_number = 1;
  while (1) {
    vexTouchDataGet(&status);
    if ((status.lastEvent == kTouchEventPress)
    && (last_press_count) != status.pressCount) {
      page_number++;
      if (page_number > 4)  page_number = 1;
    }
    last_press_count = status.pressCount;

    switch (page_number) {
      case 1:
      displayScreen_competition();
      break;

      case 2:
      displayScreen_currentLimit();
      break;

      case 3:
      displayScreen_timing();
      break;

      case 4:
      displayScreen_turret();
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


//
// Screens
//

/**
 * @brief Display competition relevant info
 * 
 */
void displayScreen_competition() {
  vexDisplayForegroundColor(ClrWhite);

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
    vexDisplayString(2, "5 Left safe route");
  } else if (auto_select < 3000) {
    vexDisplayString(2, "6 Right safe route");
  } else if (auto_select < 3500) {
    vexDisplayString(2, "7 Left F-off");
  } else if (auto_select < 4000) {
    vexDisplayString(2, "8 Right F-off");
  } else if (auto_select < 4500) {
    vexDisplayString(2, "9 ...");
  } else if (auto_select < 5000) {
    vexDisplayString(2, "10 ...");
  } else {
    vexDisplayString(2, "No auto selected");
  }
  
  // Update SBF data?
  // vexDisplayString(4, "ENC1,2,3=%u,%u,%u", sbf_data.ENC1, sbf_data.ENC2, sbf_data.ENC3);
  // vexDisplayString(4, "RAWx,y,theta=%u,%u,%u", sbf_pose.x, sbf_pose.y, sbf_pose.theta);

  // int32_t offset_xy = 10000000; // 1000m --> [tenths of mm]
  // int32_t offset_theta = 1000 * 3600;  // 1000 rotations --> [tenths of deg]
  // vexDisplayString(4, "POSEx,y,theta=%d, %d, %d",
  //   ((int32_t)sbf_pose.x - offset_xy), 
  //   ((int32_t)sbf_pose.y - offset_xy), 
  //   ((int32_t)sbf_pose.theta - offset_theta));

  vexDisplayString(3, "p_start = %.2f, %.2f, %.2f",
    p_start.x,
    p_start.y,
    p_start.theta);
  vexDisplayString(4, "p_global x,y,theta= %.2f, %.2f, %.2f",
    p_global.x,
    p_global.y,
    p_global.theta);

  vexDisplayString(6, "Heading: %f", driveGetHeading());
  vexDisplayString(7, "Distance: %f", driveGetDistance());

  vexDisplayString(8, "Disc 1: %d", intakeGetDetectorReading(1));
  vexDisplayString(9, "Disc 2: %d", intakeGetDetectorReading(2));
  vexDisplayString(10, "Disc 3: %d", intakeGetDetectorReading(3));
  // vexDisplayString(11, "Disc staging: %d", intakeGetDetectorReading(4));
  vexDisplayString(11, "Turret discs: %u", launcherFlickCountDiscs());

  // vexDisplayString(8, "Launcher RPM L: %f", launcher_avg_RPM_L);
  // vexDisplayString(9, "Launcher RPM R: %f", launcher_avg_RPM_R);
  // vexDisplayString(10, "Turret angle: %f", turretGetAngle());
  // vexDisplayString(11, "Turret value: %d", turretGetRawReading());

  // vexDisplayString(12, "Auto select: %d", miscGetAutoSelect());
}

/**
 * @brief Display V5 Smart motor current limit information for all 20 ports.
 * 
 * Line will turn red if current limit flag is on.
 */
void displayScreen_currentLimit() {
  // vexDisplayString(0, "Motor current limits: [index] limit, actual");
  int32_t total_current_mA = 0;
  int32_t total_limit_mA = 0;
  for (uint32_t i = 0; i < 20; i++) {
    // Get motor info
    int32_t motor_current = vexMotorCurrentGet(i);
    int32_t motor_current_limit = vexMotorCurrentLimitGet(i);
    bool flag_current_limit = vexMotorCurrentLimitFlagGet(i);

    // Display motor info
    int xpos, ypos;
    if (i < 10) {
      // Left column
      xpos = 0;
      ypos = i*20+30;
    } else {
      // Right column
      xpos = 250;
      ypos = (i-10)*20+30;
    }

    // Turn line red if current limit flag is on
    if (flag_current_limit) {
      vexDisplayForegroundColor(ClrRed);
    } else {
      vexDisplayForegroundColor(ClrWhite);
    }
      
    vexDisplayStringAt(xpos, ypos, "[%d] %d, %d", i, motor_current_limit, motor_current);
    total_current_mA += motor_current;
    total_limit_mA += motor_current_limit;
  }
  vexDisplayCenteredString(0, "%d of %d [mA]", total_current_mA, total_limit_mA);
}

/**
 * @brief Display timing info for various threads
 * 
 */
void displayScreen_timing() {
  vexDisplayString(0, "Main loop timing [us]");
  
  // Main superloop thread
  vexDisplayString(1, "Execution: %d", main_execution_time_us);
  vexDisplayString(2, "Yield: %d", main_yield_time_us);
  vexDisplayString(3, "Total: %d", main_execution_time_us + main_yield_time_us);

  // Display thread

  // Controls?
}

/**
 * @brief Displays turret states (turret postion, launcher speeds)
 * 
 */
void displayScreen_turret() {
  vexDisplayCenteredString(0, "Turret");
  if (miscGetJumperID()) {
    vexDisplayString(1, "Calibration ID1: L=%d, R=%d", TURRET_LEFT_READING_ID1, TURRET_RIGHT_READING_ID1);
  } else {
    vexDisplayString(1, "Calibration ID0: L=%d, R=%d", TURRET_LEFT_READING_ID0, TURRET_RIGHT_READING_ID0);
  }
  vexDisplayString(2, "Raw potentiometer [raw]: %d", turretGetRawReading());
  vexDisplayString(3, "Angle [deg]: %.2f", turretGetAngle());
  
  vexDisplayCenteredString(6, "Launcher");
  vexDisplayString(7, "Avg RPM L: %f", launcher_avg_RPM_L);
  vexDisplayString(8, "Avg RPM R: %f", launcher_avg_RPM_R);
}