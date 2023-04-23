#ifndef ROBOT_INTAKE_H
#define ROBOT_INTAKE_H

#include "v5_apitypes.h"

#define THRESHOLD_DISC_1  2000
#define THRESHOLD_DISC_2  2000
#define THRESHOLD_DISC_3  2000
#define THRESHOLD_DISC_STAGING  2000

// #define THRESHOLD_DISC_1_ID1  2000
// #define THRESHOLD_DISC_2_ID1  2000
// #define THRESHOLD_DISC_3_ID1  2000

void intakeInit(void);
void intakePeriodic(void);
void intakeDisable(void);

void intakeSpin(int32_t pwm_value);
void intakeDeploy(bool deploy);
void intakeMatchLoad(bool load);
void intakeTurretLoad(bool load_turret);
void intakeTurretLoadSequence(void);
void intakeTurretFlaps(bool hold);

int32_t intakeGetDetectorReading(uint8_t detector_number);
bool intakeDiscDetected(uint8_t detector_number);
uint8_t intakeCountDiscs();

#endif // ROBOT_INTAKE_H
