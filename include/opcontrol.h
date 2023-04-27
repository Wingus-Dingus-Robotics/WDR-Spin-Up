#ifndef OPCONTROL_H
#define OPCONTROL_H

// #include "v5_apitypes.h"
#include "robot/comms.h"

// opcontrol states
// shared with turret.cpp
extern bool state_aimbot;
extern bool state_string_aimbot;

void opcontrolInit(void);
void opcontrolPeriodic(void);

V5Cmd_t opcontrolLED(void);
V5Cmd_t opcontrolFlash(void);

#endif // OPCONTROL_H
