#include "util/control.h"

/*
 * PID motor feedback control
 */

void controlPID_init(PID_Controller_t *PID, double kP, double kI, double kD, double integral_windup_guard, double dt)
{
  // Arguments
  PID->kP = kP;   // equivalent to (*PID).kP = kP
  PID->kI = kI;
  PID->kD = kD;
  PID->integral_windup_guard = integral_windup_guard;

  // Control loop timestep in [s]
  PID->dt = dt;

  // Default range (-127 to 127 PWM value)
  PID->output_range_low_pwm = -127;
  PID->output_range_high_pwm = 127;

  // Reset states
  controlPID_resetStates(PID);  //Note that PID is already a pointer to a PID_Controller_t.
}

void controlPID_resetStates(PID_Controller_t *PID)
{
  // Reset calculation terms
  PID->error = 0;
  PID->error_prev = 0;
  PID->integral = 0;
  PID->derivative = 0;

  // Reset other states
  // PID->enable_flag = false;
  PID->target_value = 0;
  PID->current_value = 0;
  PID->output = 0;
  PID->output_pwm = 0;
}

void controlPID_setOutputRange(PID_Controller_t *PID, int32_t low, int32_t high)
{
  PID->output_range_low_pwm = low;
  PID->output_range_high_pwm = high;
}

void controlPID_calculation(PID_Controller_t *PID, double current_value)
{
  // Proportional calculation (error)
  PID->current_value = current_value;
  PID->error = PID->target_value - PID->current_value;

  // Integral calculation (cumulative error)
  PID->integral += PID->error * PID->dt;

  if (PID->integral > PID->integral_windup_guard)       // Guard high
    PID->integral = PID->integral_windup_guard;
  else if (PID->integral < -PID->integral_windup_guard) // Guard low
    PID->integral = -PID->integral_windup_guard;

  // Derivative calculation (error rate of change)
  PID->derivative = (PID->error - PID->error_prev) / PID->dt;
  PID->error_prev = PID->error;

  // PID output
  PID->output = (PID->kP * PID->error) + (PID->kI * PID->integral) + (PID->kD * PID->derivative);
  PID->output_pwm = PID->output;

  // Clip max/min output
  if (PID->output_pwm > PID->output_range_high_pwm)
    PID->output_pwm = PID->output_range_high_pwm;
  else if (PID->output_pwm < PID->output_range_low_pwm)
    PID->output_pwm = PID->output_range_low_pwm;
}