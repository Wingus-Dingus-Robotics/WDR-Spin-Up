#ifndef UTIL_CONTROL_H
#define UTIL_CONTROL_H

#include "v5_apitypes.h"

typedef struct {
  // Controller parameters
  double kP;                     //
  double kI;                     //
  double kD;                     //
  double integral_windup_guard;  // saturation limit for integral term
  double dt;                     // timestep [s]
  
  // Output range
  int32_t output_range_high_pwm;
  int32_t output_range_low_pwm;

  // PID calculation terms
  double error;        // difference target - current
  double error_prev;   // used for derivative
  double integral;     // cumulative sum of error
  double derivative;   // rate of change of error

  // Controller states
  double target_value;
  double current_value;
  double output;             // Output: Actual output of PID calculation
  int32_t output_pwm;       // Output: PWM value between -127 to 127 to input to motor PWM control.
} PID_Controller_t;

// Associated PID functions
void controlPID_init(PID_Controller_t *PID, double kP, double kI, double kD, double integral_windup_guard, double dt);
void controlPID_resetStates(PID_Controller_t *PID);
void controlPID_setOutputRange(PID_Controller_t *PID, int32_t low, int32_t high);
void controlPID_calculation(PID_Controller_t *PID, double current_value);

#endif // UTIL_CONTROL_H
