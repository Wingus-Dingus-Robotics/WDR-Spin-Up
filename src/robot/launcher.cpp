#include "robot/launcher.h"
#include "wdr.h"

// Globals
double launcher_avg_RPM_L;
double launcher_avg_RPM_R;

// Launcher presets (global constants)
launcher_speeds_t LAUNCHER_SPEED_LOW = {2550, 1700};
launcher_speeds_t LAUNCHER_SPEED_MED = {3200, 1700};
launcher_speeds_t LAUNCHER_SPEED_HIGH = {3500, 2000};
launcher_speeds_t LAUNCHER_SPEED_WALL = {2700, 1700};

// Encoder data
// raw_encoder_t raw_L_prev = {0}, raw_L = {0};
// raw_encoder_t raw_R_prev = {0}, raw_R = {0};
double launcher_L_RPM_buffer[ RPM_BUFFER_SIZE ] = {0};
double launcher_R_RPM_buffer[ RPM_BUFFER_SIZE ] = {0};

// Motor PWM and ramping control
static int32_t left_pwm_actual = 0, right_pwm_actual = 0;
static bool launcher_ramp_flag = true;

// Speed control
PID_Controller_t speed_pid_L, speed_pid_R;

// Flick sequence
static bool launcher_flick_sequence_on = false;
static bool launcher_flick_state_out = false;
static vex::timer flick_timer = vex::timer();
static uint8_t launcher_discs_loaded = 0;

// Devices
static VEX_DEVICE_GET(motor_launcher_L, port_to_index( PORT_LAUNCHER_L ) );
static VEX_DEVICE_GET(motor_launcher_R, port_to_index( PORT_LAUNCHER_R ) );
static VEX_DEVICE_GET(adix_upper_device, port_to_index( PORT_ADIX_UPPER ));
static uint32_t solenoid_flick = port_to_index( ADIX_UPPER_LAUNCH_FLICK );
static VEX_DEVICE_GET(adi_device, port_to_index( PORT_ADI ));
static uint32_t switch_lift_piston_extended = port_to_index( ADI_LIFT_PISTON_EXTENDED );

void launcherInit() {
  wdr_motor_settings_t launcher_settings = {
    .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
    .brake_mode = kV5MotorBrakeModeCoast,
    .encoder_unit = kMotorEncoderCounts,
    .gearset = kMotorGearSet_06,            // lol it's actually 1:1, not 6:1
    .reverse = false
  };
  wdrMotorInit(motor_launcher_R, launcher_settings);
  launcher_settings.reverse = true;
  wdrMotorInit(motor_launcher_L, launcher_settings);

  vexDeviceAdiPortConfigSet(adix_upper_device, solenoid_flick, kAdiPortTypeDigitalOut);
  launcherFlick(false);

  vexDeviceAdiPortConfigSet(adi_device, switch_lift_piston_extended, kAdiPortTypeDigitalIn);

  controlPID_init(&speed_pid_L, LAUNCHER_KP, LAUNCHER_KI, LAUNCHER_KD, LAUNCHER_WINDUP, LAUNCHER_DT);
  controlPID_init(&speed_pid_R, LAUNCHER_KP, LAUNCHER_KI, LAUNCHER_KD, LAUNCHER_WINDUP, LAUNCHER_DT);
  controlPID_setOutputRange(&speed_pid_L, 0, 127);
  controlPID_setOutputRange(&speed_pid_R, 0, 127);
  controlPID_resetStates(&speed_pid_L);
  controlPID_resetStates(&speed_pid_R);

  left_pwm_actual = 0;
  right_pwm_actual = 0;

  // TODO: Reset encoder values back to zero? When does rollover occur?
}

void launcherPeriodic() {
  // Speed controller, with current limiting
  int32_t pwm_left, pwm_right;
  launcherUpdateAvgSpeed();
  controlPID_calculation(&speed_pid_L, launcher_avg_RPM_L);
  controlPID_calculation(&speed_pid_R, launcher_avg_RPM_R);

  if (speed_pid_L.target_value == 0) {
    pwm_left = 0;
    vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_L ), 100);
  } else {
    pwm_left = speed_pid_L.output_pwm;
    if (fabs(speed_pid_L.error) < 100) {
      // Maintain speed
      vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_L ), 1000);
    } else {
      // Accelerate with full throttle
      vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_L ), 2500);
    }
  }

  if (speed_pid_R.target_value == 0) {
    pwm_right = 0;
    vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_R ), 100);
  } else {
    pwm_right = speed_pid_R.output_pwm;
    if (fabs(speed_pid_R.error) < 100) {
      // Maintain speed
      vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_R ), 1000);
    } else {
      // Accelerate with full throttle
      vexMotorCurrentLimitSet(port_to_index( PORT_LAUNCHER_R ), 2500);
    }
  }

  if (launcher_ramp_flag) {
    launcherSetPWMRamp(pwm_left, pwm_right);
  } else {
    launcherSetPWM(pwm_left, pwm_right);
  }

  // Flick sequence
  if (launcher_flick_sequence_on) {
    // Time before pulling flicker back in (time spent out)
    if (launcher_flick_state_out) {
      if (flick_timer.time() > 100) {
        launcherFlick(false);
        flick_timer.reset();
      }
    }
    // Time before pushing flicker back out (time spent in)
    else {
      if (flick_timer.time() > 200) {
        launcherFlick(true);
        flick_timer.reset();
      }
    }
  }
}

void launcherDisable() {
  launcherSetPWM(0, 0);
  controlPID_resetStates(&speed_pid_L);
  controlPID_resetStates(&speed_pid_R);
}

void launcherFlick(bool flick) {
  if (flick) {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_flick, 1);
    launcher_flick_state_out = true;
    if (launcher_discs_loaded > 0) {
      launcher_discs_loaded--;
    }
  } else {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_flick, 0);
    launcher_flick_state_out = false;
  }
}

void launcherFlickSequence(bool sequence_on) {
  if (sequence_on && (!launcher_flick_sequence_on)) {
    launcher_flick_sequence_on = true;
    launcherFlick(true);
    flick_timer.reset();
  } else if ((!sequence_on) && launcher_flick_sequence_on) {
    launcher_flick_sequence_on = false;
    launcherFlick(false);
  }
}

void launcherFlickSetDiscs(uint8_t n_discs) {
  launcher_discs_loaded = n_discs;
}

uint8_t launcherFlickCountDiscs() {
  return launcher_discs_loaded;
}

void launcherSetPWM(int32_t left_pwm, int32_t right_pwm) {
  if (left_pwm > 127)       left_pwm = 127;
  else if (left_pwm < 0) left_pwm = 0;
  if (right_pwm > 127)        right_pwm = 127;
  else if (right_pwm < 0)  right_pwm = 0;

  wdrMotorSetPwm(motor_launcher_L, left_pwm);
  wdrMotorSetPwm(motor_launcher_R, right_pwm);

  // Set state variables
  left_pwm_actual = left_pwm;
  right_pwm_actual = right_pwm;
}

void launcherSetPWMRamp(int32_t left_pwm_target, int32_t right_pwm_target) {
  int32_t left_pwm, right_pwm;

  // Hack to stop launcher from breaking after disconnect
  left_pwm_actual = vexDeviceMotorPwmGet(motor_launcher_L);
  right_pwm_actual = vexDeviceMotorPwmGet(motor_launcher_R);

  if (left_pwm_target > left_pwm_actual) {
    left_pwm = left_pwm_actual + LAUNCHER_RAMP_INCREMENT_PWM;
  } else if (left_pwm_target < left_pwm_actual) {
    left_pwm = left_pwm_actual - LAUNCHER_RAMP_INCREMENT_PWM;
  } else {
    left_pwm = left_pwm_actual;
    // TODO: Ramp on target flag?
  }

  if (right_pwm_target > right_pwm_actual) {
    right_pwm = right_pwm_actual + LAUNCHER_RAMP_INCREMENT_PWM;
  } else if (right_pwm_target < right_pwm_actual) {
    right_pwm = right_pwm_actual - LAUNCHER_RAMP_INCREMENT_PWM;
  } else {
    right_pwm = right_pwm_actual;
    // TODO: Ramp on target flag?
  }

  launcherSetPWM(left_pwm, right_pwm);
}

void launcherSetRPM(double left_RPM_target, double right_RPM_target) {
  speed_pid_L.target_value = left_RPM_target;
  speed_pid_R.target_value = right_RPM_target;
}

// void launcherGetRawEncoders(raw_encoder_t *left, raw_encoder_t *right) {
//   uint32_t timestamp, system_time;
//   left->value = wdrMotorGetRawEncoder(motor_launcher_L, &timestamp);
//   system_time = vexSystemHighResTimeGet();
//   left->timestamp_us = system_time;
//   // left->timestamp_ms = timestamp;

//   right->value = wdrMotorGetRawEncoder(motor_launcher_R, &timestamp);
//   system_time = vexSystemHighResTimeGet();
//   right->timestamp_us = system_time;
//   // right->timestamp_ms = timestamp;
// }

// double launcherCountsToRev(int32_t encoder_value) {
//   return encoder_value / (double)LAUNCHER_ENC_TO_REV;
// }

// void launcherGetSpeeds(double *left_RPM, double *right_RPM) {
//   // Get timestamped encoder values
//   raw_L_prev = raw_L;
//   raw_R_prev = raw_R;
//   launcherGetRawEncoders(&raw_L, &raw_R);

//   // Calculate gradient, add to RPM results buffer
//   double delta_rev_L = launcherCountsToRev( raw_L.value - raw_L_prev.value );
//   double delta_rev_R = launcherCountsToRev( raw_R.value - raw_R_prev.value );
//   double delta_min_L = (raw_L.timestamp_ms - raw_L_prev.timestamp_ms) / 1000.0 / 60.0;
//   double delta_min_R = (raw_R.timestamp_ms - raw_R_prev.timestamp_ms) / 1000.0 / 60.0;
//   double latest_RPM_L = delta_rev_L / delta_min_L;
//   double latest_RPM_R = delta_rev_R / delta_min_R;

//   for (int i=RPM_BUFFER_SIZE-1; i>0; i--) {
//     launcher_L_RPM_buffer[i] = launcher_L_RPM_buffer[i-1];
//     launcher_R_RPM_buffer[i] = launcher_R_RPM_buffer[i-1];
//   }
//   launcher_L_RPM_buffer[0] = latest_RPM_L;
//   launcher_R_RPM_buffer[0] = latest_RPM_R;

//   // Find average RPM of readings in buffer
//   double sum_L = 0;
//   double sum_R = 0;
//   for (int i=0; i<RPM_BUFFER_SIZE; i++) {
//     sum_L += launcher_L_RPM_buffer[i];
//     sum_R += launcher_R_RPM_buffer[i];
//   }
//   *left_RPM = sum_L / (double)RPM_BUFFER_SIZE;
//   *right_RPM = sum_R / (double)RPM_BUFFER_SIZE;
// }

void launcherUpdateAvgSpeed() {
  // // Get timestamped encoder values
  // raw_L_prev = raw_L;
  // raw_R_prev = raw_R;
  // launcherGetRawEncoders(&raw_L, &raw_R);

  // // Calculate gradient, add to RPM results buffer
  // double delta_rev_L = launcherCountsToRev( raw_L.value - raw_L_prev.value );
  // double delta_rev_R = launcherCountsToRev( raw_R.value - raw_R_prev.value );
  // double delta_min_L = (raw_L.timestamp_us - raw_L_prev.timestamp_us) / 1000000.0 / 60.0;
  // double delta_min_R = (raw_R.timestamp_us - raw_R_prev.timestamp_us) / 1000000.0 / 60.0;
  // double latest_RPM_L = delta_rev_L / delta_min_L;
  // double latest_RPM_R = delta_rev_R / delta_min_R;

  double latest_RPM_L = wdrMotorGetVelocity(motor_launcher_L) * 6;
  double latest_RPM_R = wdrMotorGetVelocity(motor_launcher_R) * 6;

  for (int i=RPM_BUFFER_SIZE-1; i>0; i--) {
    launcher_L_RPM_buffer[i] = launcher_L_RPM_buffer[i-1];
    launcher_R_RPM_buffer[i] = launcher_R_RPM_buffer[i-1];
  }
  launcher_L_RPM_buffer[0] = latest_RPM_L;
  launcher_R_RPM_buffer[0] = latest_RPM_R;

  // Find average RPM of readings in buffer
  double sum_L = 0;
  double sum_R = 0;
  for (int i=0; i<RPM_BUFFER_SIZE; i++) {
    sum_L += launcher_L_RPM_buffer[i];
    sum_R += launcher_R_RPM_buffer[i];
  }
  launcher_avg_RPM_L = sum_L / (double)RPM_BUFFER_SIZE;
  launcher_avg_RPM_R = sum_R / (double)RPM_BUFFER_SIZE;
}

bool launcherIsLifterUp(void) {
  return vexDeviceAdiValueGet(adi_device, switch_lift_piston_extended);
}