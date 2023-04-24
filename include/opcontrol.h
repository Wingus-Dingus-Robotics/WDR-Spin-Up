#ifndef OPCONTROL_H
#define OPCONTROL_H

// #include "v5_apitypes.h"
#include "robot/comms.h"

void opcontrolInit(void);
void opcontrolPeriodic(void);

V5Cmd_t opcontrolLED(void);
V5Cmd_t opcontrolFlash(void);

#endif // OPCONTROL_H
