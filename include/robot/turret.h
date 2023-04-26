#ifndef ROBOT_TURRET_H
#define ROBOT_TURRET_H

#include "v5_apitypes.h"

#define TURRET_LEFT_READING_ID0   2716
//Straight ~= 2096
#define TURRET_RIGHT_READING_ID0  1488  //1488
#define TURRET_LEFT_READING_ID1   2711
#define TURRET_RIGHT_READING_ID1  1470

#define TURRET_PID_KP     7.0
#define TURRET_PID_KI     0.0
#define TURRET_PID_WINDUP 0.0
#define TURRET_PID_KD     0.03
#define TURRET_PID_DT     0.001

#define TURRET_MAX_PWM    127     //80

#define TURRET_SETTLING_TIME_MS         500
#define TURRET_SETTLING_THRESHOLD_DEG   5.0

void turretInit(void);
void turretPeriodic(void);
void turretDisable(void);

void turretBrake(bool brake_on);

void turretSpinPWM(int32_t pwm_value);
int32_t turretGetRawReading(void);
double turretGetAngle(void);

void turretSetAngle(double angle_deg);
double turretGetTargetAngle(void);

void turretRollerSpinPWM(int32_t pwm_value);

#endif // ROBOT_TURRET_H
