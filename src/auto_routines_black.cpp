#include "auto_routines_black.h"
#include "auto_routines_white.h"
#include "wdr.h"

using namespace vex;

Pose2D_t black_autoGetStartingPose(void) {
  return white_autoGetStartingPose();
  // Pose2D_t pose;
  // uint32_t auto_select = miscGetAutoSelect();
  // if (auto_select < 500) {
  //   // Autonomous 1
  //   pose = {1800,1800,0};
  // } else if (auto_select < 1000) {
  //   // Autonomous 2
  //   pose = {0,0,0};
  // } else if (auto_select < 1500) {
  //   // Autonomous 3
  //   pose = {0,0,0};
  // } else if (auto_select < 2000) {
  //   // Autonomous 4
  //   pose = {0,0,0};
  // } else if (auto_select < 2500) {
  //   // Autonomous 5
  //   pose = {0,0,0};
  // } else if (auto_select < 3000) {
  //   // Autonomous 6
  //   pose = {0,0,0};
  // } else if (auto_select < 3500) {
  //   // Autonomous 7
  //   pose = {0,0,0};
  // } else if (auto_select < 4000) {
  //   // Autonomous 8
  //   pose = {0,0,0};
  // } else {
  //   // Default
  //   pose = {0,0,0};
  // }
  // return pose;
}

/**
 * @brief Drive tuning
 * 
 * Power/velocity values from here are probably okay to use.
 */
int black_autonomous1(void) {
  white_autonomous1();

  return 0;
}

int black_autonomous2(void) {
  white_autonomous2();

  return 0;
}

int black_autonomous3(void) {
  white_autonomous3();
  return 0;
}

int black_autonomous4(void) {
  white_autonomous4();
  return 0;
}

int black_autonomous5(void) {
  white_autonomous5();
  return 0;
}

int black_autonomous6(void) {
  white_autonomous6();
  return 0;
}

int black_autonomous7(void) {
  white_autonomous7();
  return 0;
}

int black_autonomous8(void) {
  white_autonomous8();
  return 0;
}

int black_autonomous9(void) {
  white_autonomous9();
  return 0;
}

int black_autonomous10(void) {
  white_autonomous10();
  return 0;
}