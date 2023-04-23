/**
 * @file drive.h
 * @author your name (you@domain.com)
 * @brief 
 * @date 2023-02-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DRIVE_H
#define DRIVE_H

#include "v5_apitypes.h"

// Profiled move PID constants
#define DRIVE_PROFILE_DISTANCE_KP       0.7
#define DRIVE_PROFILE_DISTANCE_KI       0.0
#define DRIVE_PROFILE_DISTANCE_KD       0.0
#define DRIVE_PROFILE_DISTANCE_WINDUP   50.0
#define DRIVE_PROFILE_DISTANCE_SETTLING_RANGE_MM  50.0

#define DRIVE_PROFILE_INNER_DT    0.001
#define DRIVE_PROFILE_OUTER_DT    0.5

// Basic move PID constants
#define DRIVE_PID_DISTANCE_KP       0.7
#define DRIVE_PID_DISTANCE_KI       0.1
#define DRIVE_PID_DISTANCE_KD       0.003
#define DRIVE_PID_DISTANCE_WINDUP   50.0
#define DRIVE_PID_DISTANCE_SETTLING_RANGE_MM  20.0

#define DRIVE_PID_ROTATION_KP       5.0
#define DRIVE_PID_ROTATION_KI       5.0
#define DRIVE_PID_ROTATION_KD       0.001
#define DRIVE_PID_ROTATION_WINDUP   10.0
#define DRIVE_PID_ROTATION_SETTLING_ANGLE_DEG 5.0

#define DRIVE_PID_DT    0.001

// Ramping
#define RAMPING_INCREMENT_PWM   1

// Distance measurement
#define DRIVE_ENC_RESOLUTION  8192
#define DRIVE_ODOM_CIRC_MM    200

void driveInit(void);
void drivePeriodic(void);
void driveDisable(void);

// Movement functions
void driveSetPWM(int32_t left_pwm, int32_t right_pwm);
void driveSetPWMRamp(int32_t left_pwm_target, int32_t right_pwm_target);
void driveSetPct(double left, double right);

void driveMoveDistance(double distance_mm, int32_t max_pwm, uint32_t timeout_ms);
void driveTurnAngle(double angle_deg, int32_t max_pwm, uint32_t timeout_ms);
// void driveMoveArc(double max_power, double angle_deg, double radius_cm, int32_t timeout_ms)

void driveProfileDistance(double distance_mm, double max_acceleration, double max_velocity, uint32_t timeout_ms);
void driveProfileAngle(double angle_deg, double max_acceleration, double max_velocity, uint32_t timeout);

// Sensor functions
void driveResetHeading(void);
double driveGetHeading(void);
void driveResetDistance(void);
double driveGetDistance(void);

#endif // DRIVE_H
