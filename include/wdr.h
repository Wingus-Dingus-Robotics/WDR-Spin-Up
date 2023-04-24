/**
 * @file wdr.h
 * @author your name (you@domain.com)
 * @brief Project-wide includes, macros and globals that should be accessible from any module in the project.
 *        This file is analagous to vex.h in a typical VEXcode C++ project.
 * @date  2022-12-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// System headers
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// V5 C API
#include "v5_api.h"
#include "v5_apiprivate.h"
#include "v5_apitypes.h"
#include "v5_apiuser.h"     // Generic serial comms in here

// // VEXcode C++ API for multitasking
// #include "vex_task.h"
// #include "vex_thread.h"
// #include "vex_event.h"    // Callback event handler
// #include "vex_mevent.h"

// VEXcode C++ API (all, can't really cherry pick specific modules)
#include "v5_cpp.h"

// WINGU5 Drivers for low-level C API device access
#include "wdr/competition.h"
#include "wdr/controller.h"
#include "wdr/motor.h"
#include "wdr/serial.h"
#include "wdr/timer.h"

// WINGU5 robot subsystems
#include "robot/comms.h"
#include "robot/drive.h"
#include "robot/intake.h"
#include "robot/launcher.h"
#include "robot/misc.h"
#include "robot/odom.h"
#include "robot/turret.h"

// Other useful utilities
#include "util/control.h"

// Useful macros
// #define max(a, b) ((a > b) ? a : b)
// #define min(a, b) ((a < b) ? a : b)
#define abs(a) ((a < 0) ? -a : a)

// List of global variable declarations
// Any file that includes wdr.h can access these.
extern sbf_data_t sbf_data; // Encoder data received from SBF coprocessor (NOW UNUSED.)
extern sbf_pose_t sbf_pose; // Pose data received from SBF coprocessor
// extern Pose2D_t global_pose;  // Global pose of robot on field
extern double launcher_avg_RPM_L, launcher_avg_RPM_R;
extern bool misc_jumper_id;       // Robot ID jumper, misc.cpp
extern uint64_t main_execution_time_us, main_yield_time_us;   // Timing, main.cpp

extern Pose2D_t p_sbf, p_global, p_start;  // Robot odometry

/*
 *  Port macros
 */

#define port_to_index(port)   (port - 1)

// SBF coprocessor
#define PORT_SBF_TX   15  // V5 to SBF UART Tx (out), LiDAR power
#define PORT_SBF_RX   16  // SBF to V5 UART Rx (in), SBF power

// Debug serial
#define PORT_DEBUG    6   // Serial debug out

// 3-wire ports
#define PORT_ADI          22  // V5 Brain 3-wire ADI
#define PORT_ADIX_LOWER   10  // 3-wire expander
#define PORT_ADIX_UPPER   3   // 3-wire expander on turret

#define ADI_LIFT_PISTON 1   // ?
#define ADI_PRESSURE    2   // Pressure sensor
#define ADI_DISC_STAGING 3  // Line tracker

#define ADI_AUTO_SELECT 4   // Potentiometer
#define ADI_TURRET_POS  5   // Potentiometer
#define ADI_DISC_3      6   // Line tracker
#define ADI_DISC_2      7   // Line tracker
#define ADI_DISC_1      8   // Line tracker

//TODO: Document lower adix ports
#define ADIX_LOWER_INTAKE_MATCH_LOAD  1 // Pneumatic solenoid
#define ADIX_LOWER_INTAKE_FLAP_R  2 // Pneumatic solenoid
#define ADIX_LOWER_INTAKE_DEPLOY  3 // Pneumatic solenoid
#define ADIX_LOWER_INTAKE_FLAP_L  4 // Pneumatic solenoid
#define ADIX_LOWER_INTAKE_LIFTER  5 // Pneumatic solenoid, lifts discs into turret
#define ADIX_LOWER_TURRET_BRAKE   6 // Pneumatic solenoid

#define ADIX_UPPER_LAUNCH_FLICK     1   // Pneumatic solenoid, flick disc into launcher
#define ADIX_UPPER_STRING_1         2   // Pneumatic solenoid
#define ADIX_UPPER_STRING_2         3   // Pneumatic solenoid
#define ADIX_UPPER_ROLLER_ROTATION  4   // Roller rotation sensor
#define ADIX_UPPER_JUMPER_ID        8 // Robot ID jumper (in == white/15")

// Motors
#define PORT_LAUNCHER_L 1
#define PORT_LAUNCHER_R 2
#define PORT_ROLLER     5
#define PORT_INTAKE_L   7
#define PORT_INTAKE_R   8
#define PORT_TURRET     9
#define PORT_DRIVE_L_FT 11
#define PORT_DRIVE_L_FB 12
#define PORT_DRIVE_L_RT 13
#define PORT_DRIVE_L_RB 14
#define PORT_DRIVE_R_RB 17
#define PORT_DRIVE_R_RT 18
#define PORT_DRIVE_R_FB 19
#define PORT_DRIVE_R_FT 20

// Other sensors
#define PORT_IMU  4