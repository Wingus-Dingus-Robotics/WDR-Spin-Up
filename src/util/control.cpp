#include "util/control.h"
#include <math.h>

/*
 * PID motor feedback control
 */

void controlPID_init(PID_Controller_t *PID, double kP, double kI, double kD, double integral_windup_guard, double dt)
{
  // Arguments
  PID->kF = 0.0;
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
  PID->ff_value = 0;
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

/* Optional feedforward */

void controlPID_setFeedForward(PID_Controller_t *PID, double kF) {
  PID->kF = kF;
}

void controlPID_calculationFeedForward(PID_Controller_t *PID, double current_value, double ff_value) {
  // Feedforward calculation
  double ff_output = ff_value * PID->kF;

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
  PID->output = ff_output + (PID->kP * PID->error) + (PID->kI * PID->integral) + (PID->kD * PID->derivative);
  PID->output_pwm = PID->output;

  // Clip max/min output
  if (PID->output_pwm > PID->output_range_high_pwm)
    PID->output_pwm = PID->output_range_high_pwm;
  else if (PID->output_pwm < PID->output_range_low_pwm)
    PID->output_pwm = PID->output_range_low_pwm;
}

/******************
 * Motion profile *
 ******************/

/**
 * @brief Initialize a trapezoidal motion profile
 *
 * Assuming that initial position and initial states are 0.
 *
 * For straight line, position <unit> would be [mm].
 *
 * For spot rotation, position <unit> would be [degrees].
 *
 * If the final position is negative, then the constraints should also be negative.
 *
 * @param profile           Pointer to profile struct
 * @param max_acceleration  Constraint for max acceleration [<unit>/s^2]
 * @param max_velocity      Constraint for max velocity [<unit>/s]
 * @param final_position    Final position [<unit>]
 */
void controlProfile_init(TrapezoidalProfile_t *profile, double max_acceleration, double max_velocity, double final_position) {
  // Constraints and goal
  profile->max_acceleration = max_acceleration;
  profile->max_velocity = max_velocity;   // TODO: Calculate true max velocity?
  profile->final_position = final_position;

  // Reset targets
  profile->target_acceleration = 0;
  profile->target_velocity = 0;
  profile->target_position = 0;

  // Reset states
  // profile->actual_position = 0;
  profile->prev_position = 0;
}

/**
 * @brief Calculate kinematic targets for the next timestep
 * 
 * @param profile         Pointer to profile struct
 * @param actual_position Sensor position [<unit>]
 * @param dt              Timestep [s]
 */
void controlProfile_update(TrapezoidalProfile_t *profile, double actual_position, double dt) {
  /* Find characteristics of motion profile */
  double t_accel, x_accel, x_cruise;

  // Eq: Time taken to accelerate [s]
  t_accel = profile->max_velocity / profile->max_acceleration;  // [s]

  // Eq: Distance covered during acceleration period [<unit>]
  x_accel = 0.5 * profile->max_acceleration * t_accel * t_accel;

  // Eq: Distance covered during cruise period [<unit>]
  // First check if there will be a cruise period.
  enum profileShape {TRAPEZOID, TRIANGLE} shape;
  if (profile->final_position < 0) {
    // Negative final position
    if (2*x_accel < profile->final_position) {
      shape = TRIANGLE;
    } else {
      shape = TRAPEZOID;
    }
  } else {
    // Positive final position
    if (2*x_accel > profile->final_position) {
      shape = TRIANGLE;
    } else {
      shape = TRAPEZOID;
    }
  }
  // Find cruise distance
  if (shape == TRIANGLE) {
    // Recalculate maximum velocity first
    t_accel = sqrt( profile->final_position / profile->max_acceleration );
    profile->max_velocity = profile->final_position / t_accel;
    x_accel = 0.5 * profile->final_position;
    x_cruise = 0;
  } else {
    // Normal trapezoidal profile
    x_cruise = profile->final_position - 2*x_accel;
  }

  /* Update setpoints */
  enum profilePeriod {ACCEL, CRUISE, DECCEL, END} period;

  // Find which point of motion profile robot is at
  if (profile->final_position < 0) {
    // Negative positions
    if (actual_position > x_accel) {
      period = ACCEL;
    } else if (actual_position > (x_accel + x_cruise)) {
      period = CRUISE;
    } else if (actual_position > (2*x_accel + x_cruise)) {
      period = DECCEL;
    } else {
      period = END;
    }
  } else {
    // Positive positions
    if (actual_position < x_accel) {
      period = ACCEL;
    } else if (actual_position < (x_accel + x_cruise)) {
      period = CRUISE;
    } else if (actual_position < (2*x_accel + x_cruise)) {
      period = DECCEL;
    } else {
      period = END;
    }
  }

  // Find target acceleration & velocity for next dt
  switch (period) {
  case ACCEL:
    profile->target_acceleration = profile->max_acceleration;
    if (fabs(profile->max_velocity - profile->target_velocity) < profile->max_acceleration*dt) {
      profile->target_velocity = profile->max_velocity;
    } else {
      profile->target_velocity += profile->max_acceleration * dt;
    }
    break;
  case CRUISE:
    profile->target_acceleration = 0;
    profile->target_velocity = profile->max_velocity;
    break;
  case DECCEL:
    profile->target_acceleration = -profile->max_acceleration;
    if (fabs(0 - profile->target_velocity) < 0) {
      profile->target_velocity = 0;
    } else {
      profile->target_velocity -= profile->max_acceleration * dt;
    }
    break;
  case END:
    profile->target_acceleration = 0;
    profile->target_velocity = 0;
    break;
  }

  // Calculate actual instantaneous velocity
  double actual_velocity = (actual_position - profile->prev_position) / dt;
  profile->prev_position = actual_position;

  // Find target position for next dt
  profile->target_position = actual_position + 0.5 * (actual_velocity + profile->target_velocity)*dt;

  // //Lookahead hack
  // if (period == ACCEL) {
  //   profile->target_position += 10;
  // }
}