#include "wdr/controller.h"
#include "v5_api.h"

// Previous button states
btn_state_t master_btn = {false}, master_btn_prev = {false};
btn_state_t partner_btn = {false}, partner_btn_prev = {false};

void controllerBtnStateUpdate(void) {
  master_btn_prev = master_btn;
  master_btn.L1 = vexControllerGet(kControllerMaster, ButtonL1);
  master_btn.L2 = vexControllerGet(kControllerMaster, ButtonL2);
  master_btn.R1 = vexControllerGet(kControllerMaster, ButtonR1);
  master_btn.R2 = vexControllerGet(kControllerMaster, ButtonR2);
  master_btn.Up = vexControllerGet(kControllerMaster, ButtonUp);
  master_btn.Down = vexControllerGet(kControllerMaster, ButtonDown);
  master_btn.Left = vexControllerGet(kControllerMaster, ButtonLeft);
  master_btn.Right = vexControllerGet(kControllerMaster, ButtonRight);
  master_btn.X = vexControllerGet(kControllerMaster, ButtonX);
  master_btn.B = vexControllerGet(kControllerMaster, ButtonB);
  master_btn.Y = vexControllerGet(kControllerMaster, ButtonY);
  master_btn.A = vexControllerGet(kControllerMaster, ButtonA);

  partner_btn_prev = partner_btn;
  partner_btn.L1 = vexControllerGet(kControllerPartner, ButtonL1);
  partner_btn.L2 = vexControllerGet(kControllerPartner, ButtonL2);
  partner_btn.R1 = vexControllerGet(kControllerPartner, ButtonR1);
  partner_btn.R2 = vexControllerGet(kControllerPartner, ButtonR2);
  partner_btn.Up = vexControllerGet(kControllerPartner, ButtonUp);
  partner_btn.Down = vexControllerGet(kControllerPartner, ButtonDown);
  partner_btn.Left = vexControllerGet(kControllerPartner, ButtonLeft);
  partner_btn.Right = vexControllerGet(kControllerPartner, ButtonRight);
  partner_btn.X = vexControllerGet(kControllerPartner, ButtonX);
  partner_btn.B = vexControllerGet(kControllerPartner, ButtonB);
  partner_btn.Y = vexControllerGet(kControllerPartner, ButtonY);
  partner_btn.A = vexControllerGet(kControllerPartner, ButtonA);
}

static bool getBtnStoredState(V5_ControllerId id, V5_ControllerIndex index, bool prev_state) {
  bool rtn_state;             // State to return
  btn_state_t *ptr_btn_state; // Pointer to joystick button global state struct

  // Select btn state struct
  if (id == kControllerPartner) {
    // Partner joystick
    if (prev_state) {
      ptr_btn_state = &partner_btn_prev;
    } else {
      ptr_btn_state = &partner_btn;
    }
    
  } else {
    // Master joystick
    if (prev_state) {
      ptr_btn_state = &master_btn_prev;
    } else {
      ptr_btn_state = &master_btn;
    }
  }

  // Get stored previous btn state
  switch (index) {
  case ButtonL1:
    rtn_state = (*ptr_btn_state).L1;
    break;
  case ButtonL2:
    rtn_state = (*ptr_btn_state).L2;
    break;
  case ButtonR1:
    rtn_state = (*ptr_btn_state).R1;
    break;
  case ButtonR2:
    rtn_state = (*ptr_btn_state).R2;
    break;
  case ButtonUp:
    rtn_state = (*ptr_btn_state).Up;
    break;
  case ButtonDown:
    rtn_state = (*ptr_btn_state).Down;
    break;
  case ButtonLeft:
    rtn_state = (*ptr_btn_state).Left;
    break;
  case ButtonRight:
    rtn_state = (*ptr_btn_state).Right;
    break;
  case ButtonX:
    rtn_state = (*ptr_btn_state).X;
    break;
  case ButtonB:
    rtn_state = (*ptr_btn_state).B;
    break;
  case ButtonY:
    rtn_state = (*ptr_btn_state).Y;
    break;
  case ButtonA:
    rtn_state = (*ptr_btn_state).A;
    break;
  default:
    rtn_state = 0;
  }

  return rtn_state;
}

//
// User functions
//

bool controllerGetBtnState(V5_ControllerId id, V5_ControllerIndex index) {
  return getBtnStoredState(id, index, false);
}

bool controllerIsBtnPressed(V5_ControllerId id, V5_ControllerIndex index) {
  bool current_state = getBtnStoredState(id, index, false);
  bool prev_state = getBtnStoredState(id, index, true);

  // Check if button has just been pressed
  if ((prev_state == false) && (current_state == true))
    return true;
  else
    return false;
}

bool controllerIsBtnReleased(V5_ControllerId id, V5_ControllerIndex index) {
  bool current_state = getBtnStoredState(id, index, false);
  bool prev_state = getBtnStoredState(id, index, true);

  // Check if button has just been released
  if ((prev_state == true) && (current_state == false))
    return true;
  else
    return false;
}