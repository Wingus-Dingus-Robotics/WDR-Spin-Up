#ifndef ROBOT_MISC_H
#define ROBOT_MISC_H

#include "v5_apitypes.h"

void miscInit(void);

void miscString(bool shoot);
uint32_t miscGetAutoSelect(void);
bool miscGetJumperID(void);

#endif // ROBOT_MISC_H
