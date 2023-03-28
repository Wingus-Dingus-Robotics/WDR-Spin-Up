#ifndef WDR_CONTROLLER_H
#define WDR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "v5_apitypes.h"

// Joystick button states
typedef struct {
  bool L1;
  bool L2;
  bool R1;
  bool R2;
  bool Up;
  bool Down;
  bool Left;
  bool Right;
  bool X;
  bool B;
  bool Y;
  bool A;
} btn_state_t;

// Button state update (called once every periodic cycle)
// We use this function instead of directly calling vexControllerGet() for buttons in driver.cpp
// so that button actions that require memory of previous state (pressed, released) can be used.
void controllerBtnStateUpdate(void);

// Helper
static bool getBtnStoredState(V5_ControllerId id, V5_ControllerIndex index, bool prev_state);

// Functions used in driver.cpp
// 'Pressing' means button is indented in down position
// 'Pressed' means button has just transitioned from Pressing=false to Pressing=true
// Vice versa for 'Released'
// TODO: Would different naming be less confusing (e.g. edge trigger?)
bool controllerGetBtnState(V5_ControllerId id, V5_ControllerIndex index);
bool controllerIsBtnPressed(V5_ControllerId id, V5_ControllerIndex index);
bool controllerIsBtnReleased(V5_ControllerId id, V5_ControllerIndex index);

// For analog joystick axes, just use vexControllerGet(kControllerMaster, Axis1)

#ifdef __cplusplus
}
#endif

#endif // WDR_CONTROLLER_H
