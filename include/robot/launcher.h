#ifndef ROBOT_LAUNCHER_H
#define ROBOT_LAUNCHER_H

#include "v5_apitypes.h"
#include "util/control.h"

#define LAUNCHER_ENC_TO_REV 50
#define LAUNCHER_RAMP_INCREMENT_PWM 1
#define RPM_BUFFER_SIZE 50

#define LAUNCHER_KP 1.5
#define LAUNCHER_KI 0.0
#define LAUNCHER_WINDUP 0.0
#define LAUNCHER_KD 0.05
#define LAUNCHER_DT 0.001

typedef struct {
  double left_RPM;
  double right_RPM;
} launcher_speeds_t;

extern launcher_speeds_t LAUNCHER_SPEED_LOW;
extern launcher_speeds_t LAUNCHER_SPEED_MED;
extern launcher_speeds_t LAUNCHER_SPEED_HIGH;
extern launcher_speeds_t LAUNCHER_SPEED_WALL;

// Hack to view PID output in display.cpp
extern PID_Controller_t speed_pid_L, speed_pid_R;

void launcherInit(void);
void launcherPeriodic(void);
void launcherDisable(void);

void launcherFlick(bool flick);
void launcherFlickSequence(bool sequence_on);

void launcherFlickSetDiscs(uint8_t n_discs);
uint8_t launcherFlickCountDiscs();

void launcherSetPWM(int32_t left_pwm, int32_t right_pwm);
void launcherSetPWMRamp(int32_t left_pwm_target, int32_t right_pwm_target);
void launcherSetRPM(double left_RPM_target, double right_RPM_target);

// typedef struct {
//   int32_t value;
//   uint32_t timestamp_us;
// } raw_encoder_t;

// void launcherGetRawEncoders(raw_encoder_t *left, raw_encoder_t *right);
// double launcherCountsToRev(int32_t encoder_value);
// // void launcherGetSpeeds(double *left_RPM, double *right_RPM);
void launcherUpdateAvgSpeed(void);

// TODO: Controller(s) for speed and disc spin control

#endif // ROBOT_LAUNCHER_H
