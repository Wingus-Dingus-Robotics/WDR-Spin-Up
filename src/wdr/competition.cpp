#include "wdr/competition.h"
#include "v5_api.h"

CompetitionMode_t competition_mode = Disabled;
CompetitionMode_t competition_mode_prev = Disabled;

//
// Static helper functions
//

static bool isCompStatusDisabled() {
  if (vexCompetitionStatus() & V5_COMP_BIT_EBL)
    return true;  // Disabled
  else
    return false; // Enabled
}
static bool isCompStatusAutonomous() {
  if (vexCompetitionStatus() & V5_COMP_BIT_MODE)
    return true;  // Autonomous
  else
    return false; // Driver
}

//
// Competition status update
//

void wdrUpdateCompStatus() {
  // Save previous state
  competition_mode_prev = competition_mode;

  // Update current state
  if (isCompStatusDisabled()) {
    competition_mode = Disabled;
  }
  else if (isCompStatusAutonomous()) {
    if ((competition_mode_prev == InitAutonomous) || (competition_mode_prev == Autonomous))
      competition_mode = Autonomous;
    else
      competition_mode = InitAutonomous;
  }
  else {
    if ((competition_mode_prev == InitDriver) || (competition_mode_prev == Driver))
      competition_mode = Driver;
    else
      competition_mode = InitDriver;
  }

  // // Return competition state
  // return competition_mode;
}

CompetitionMode_t wdrGetCompStatus() {
  return competition_mode;
}
