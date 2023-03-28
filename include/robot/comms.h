#ifndef ROBOT_COMMS_H
#define ROBOT_COMMS_H

#include "v5_apitypes.h"
#include "wdr/serial.h"

void commsInit(void);
int32_t commsUpdate(V5Cmd_t command);

#endif // ROBOT_COMMS_H
