#ifndef UTIL_CONTROL_H
#define UTIL_CONTROL_H

#include "v5_apitypes.h"

typedef struct {
  // Controller parameters
  double kF;    // Optional feedforward.
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
  double ff_value;      // For optional feedforward
  double output;             // Output: Actual output of PID calculation
  int32_t output_pwm;       // Output: PWM value between -127 to 127 to input to motor PWM control.
} PID_Controller_t;

// Associated PID functions
void controlPID_init(PID_Controller_t *PID, double kP, double kI, double kD, double integral_windup_guard, double dt);
void controlPID_resetStates(PID_Controller_t *PID);
void controlPID_setOutputRange(PID_Controller_t *PID, int32_t low, int32_t high);
void controlPID_calculation(PID_Controller_t *PID, double current_value);

// Optional feedforward term
void controlPID_setFeedForward(PID_Controller_t *PID, double kF);
void controlPID_calculationFeedForward(PID_Controller_t *PID, double current_value, double ff_value);

/* Motion profile */
// Resources:
// https://docs.wpilib.org/en/stable/docs/software/advanced-controls/controllers/trapezoidal-profiles.html
// https://www.ctrlaltftc.com/advanced/motion-profiling
// https://www.khanacademy.org/science/physics/one-dimensional-motion/kinematic-formulas/a/what-are-the-kinematic-formulas

typedef struct {
  // Constraints
  double max_acceleration;
  double max_velocity;

  // Goal
  double final_position;

  // Kinematic targets
  double target_acceleration;
  double target_velocity;
  double target_position;

  // Kinematic states
  // double actual_acceleration;
  // double actual_velocity;
  // double actual_position;
  double prev_position;

  // std::array<double, 20> actual_position_buffer;
  // std::array<double, 20> timestep_buffer;

  // Note: You could do some smart stats stuff with predicted vs actual state

} TrapezoidalProfile_t;

void controlProfile_init(TrapezoidalProfile_t *profile, double max_acceleration, double max_velocity, double final_position);
void controlProfile_update(TrapezoidalProfile_t *profile, double actual_position, double dt);

#endif // UTIL_CONTROL_H
