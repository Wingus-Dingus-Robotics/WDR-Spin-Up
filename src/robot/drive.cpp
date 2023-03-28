#include "robot/drive.h"
#include "wdr.h"

//
// Globals
//

// Motor PWM state and ramping control
// TODO: Should we work in PWM value or percent for ramping?
static int32_t left_pwm_actual = 0;
static int32_t right_pwm_actual = 0;
bool drive_ramp_flag = true;
bool drive_ramp_L_on_target_flag = true;
bool drive_ramp_R_on_target_flag = true;

// Movement controllers
static PID_Controller_t distance_pid;
static PID_Controller_t rotation_pid;
bool drive_distance_pid_flag = false;
bool drive_rotation_pid_flag = false;

// Sensor states
static int32_t origin_ENC1, origin_ENC2, origin_ENC3;
static double origin_heading;

// Devices
VEX_DEVICE_GET(motorLFT, port_to_index( PORT_DRIVE_L_FT ));
VEX_DEVICE_GET(motorLFB, port_to_index( PORT_DRIVE_L_FB ));
VEX_DEVICE_GET(motorLRT, port_to_index( PORT_DRIVE_L_RT ));
VEX_DEVICE_GET(motorLRB, port_to_index( PORT_DRIVE_L_RB ));
VEX_DEVICE_GET(motorRFT, port_to_index( PORT_DRIVE_R_FT ));
VEX_DEVICE_GET(motorRFB, port_to_index( PORT_DRIVE_R_FB ));
VEX_DEVICE_GET(motorRRT, port_to_index( PORT_DRIVE_R_RT ));
VEX_DEVICE_GET(motorRRB, port_to_index( PORT_DRIVE_R_RB ));
VEX_DEVICE_GET(sensorIMU, port_to_index( PORT_IMU ));

V5_DeviceT left_motors[4] = {motorLFT, motorLRT, motorLFB, motorLRB};   // Top, Top, Bot, Bot
V5_DeviceT right_motors[4] = {motorRFB, motorRRB, motorRFT, motorRRT};  // Bot, Bot, Top, Top

void driveInit() {
  wdr_motor_settings_t drive_settings = {
    .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
    .brake_mode = kV5MotorBrakeModeCoast,
    .encoder_unit = kMotorEncoderCounts,
    .gearset = kMotorGearSet_06,
    .reverse = false
  };

  // Left: Top motors
  // Right: Bottom motors
  for (int i=0; i<2; i++) {
    wdrMotorInit(left_motors[i], drive_settings);
    wdrMotorInit(right_motors[i], drive_settings);
  }

  // Left: Reverse Bottom motors
  // Right: Reverse Top motors
  drive_settings.reverse = true;
  for (int i=2; i<4; i++) {
    wdrMotorInit(left_motors[i], drive_settings);
    wdrMotorInit(right_motors[i], drive_settings);
  }

  // Sensors
  vexDeviceImuReset(sensorIMU);
  vex::this_thread::sleep_for(5000);
  driveResetDistance();
  driveResetHeading();

  // Control
  controlPID_init(&distance_pid, DRIVE_PID_DISTANCE_KP, DRIVE_PID_DISTANCE_KI, DRIVE_PID_DISTANCE_KD, DRIVE_PID_DISTANCE_WINDUP, DRIVE_PID_DT);
  controlPID_init(&rotation_pid, DRIVE_PID_ROTATION_KP, DRIVE_PID_ROTATION_KI, DRIVE_PID_ROTATION_KD, DRIVE_PID_ROTATION_WINDUP, DRIVE_PID_DT);
  controlPID_resetStates(&distance_pid);
  controlPID_resetStates(&rotation_pid);
}

// void drivePeriodic() {
//   // Only used in autonomous right now.

//   if (drive_distance_pid_flag) {

//   } else if (drive_rotation_pid_flag) {

//   }
// }

void driveDisable() {
  driveSetPWM(0, 0);
  controlPID_resetStates(&distance_pid);
  controlPID_resetStates(&rotation_pid);
}

void driveSetPWM(int32_t left_pwm, int32_t right_pwm)
{
  if (left_pwm > 127)       left_pwm = 127;
  else if (left_pwm < -127) left_pwm = -127;
  if (right_pwm > 127)        right_pwm = 127;
  else if (right_pwm < -127)  right_pwm = -127;

  for (int i=0; i<4; i++) {
    wdrMotorSetPwm(left_motors[i], left_pwm);
    wdrMotorSetPwm(right_motors[i], right_pwm);
  }

  // Set state variables
  left_pwm_actual = left_pwm;
  right_pwm_actual = right_pwm;
}

void driveSetPWMRamp(int32_t left_pwm_target, int32_t right_pwm_target) {
  int32_t left_pwm, right_pwm;

  if (left_pwm_target > left_pwm_actual) {
    left_pwm = left_pwm_actual + RAMPING_INCREMENT_PWM;
    drive_ramp_L_on_target_flag = false;
  } else if (left_pwm_target < left_pwm_actual) {
    left_pwm = left_pwm_actual - RAMPING_INCREMENT_PWM;
    drive_ramp_L_on_target_flag = false;
  } else {
    left_pwm = left_pwm_actual;
    drive_ramp_L_on_target_flag = true;
  }

  if (right_pwm_target > right_pwm_actual) {
    right_pwm = right_pwm_actual + RAMPING_INCREMENT_PWM;
    drive_ramp_R_on_target_flag = false;
  } else if (right_pwm_target < right_pwm_actual) {
    right_pwm = right_pwm_actual - RAMPING_INCREMENT_PWM;
    drive_ramp_R_on_target_flag = false;
  } else {
    right_pwm = right_pwm_actual;
    drive_ramp_R_on_target_flag = true;
  }

  driveSetPWM(left_pwm, right_pwm);
}

void driveSetPct(double left, double right) {
  int32_t left_pwm = left * 127;
  int32_t right_pwm = right * 127;

  // TODO Check if ramping is enabled
  if (drive_ramp_flag) {
    driveSetPWMRamp(left_pwm, right_pwm);
  } else {
    driveSetPWM(left_pwm, right_pwm);
  }
  
}

void driveResetHeading() {
  origin_heading = vexDeviceImuHeadingGet(sensorIMU);
}

double driveGetHeading(void) {
  return vexDeviceImuHeadingGet(sensorIMU) - origin_heading;
}

void driveResetDistance() {
  origin_ENC1 = sbf_data.ENC1;
  origin_ENC2 = sbf_data.ENC2;
  origin_ENC3 = sbf_data.ENC3;
}

double driveGetDistance() {
  // ENC to rotations to distance
  int32_t enc_counts = -(sbf_data.ENC1 - origin_ENC1);
  return (double)enc_counts / DRIVE_ENC_RESOLUTION * DRIVE_ODOM_CIRC_MM;
}

void driveMoveDistance(double distance_mm, int32_t max_pwm, uint32_t timeout_ms) {
  // Setup
  driveResetDistance();
  drive_distance_pid_flag = true;
  drive_rotation_pid_flag = false;
  vex::timer timeout_timer = vex::timer();
  vex::timer extra_timeout_timer = vex::timer();    // This is for movements that don't reach settling zone
  distance_pid.target_value = distance_mm;
  if (max_pwm > 0) {
    controlPID_setOutputRange(&distance_pid, -max_pwm, max_pwm);
  } else {
    controlPID_setOutputRange(&distance_pid, max_pwm, -max_pwm);
  }

  // Start move, Wait until up to full speed
  drive_ramp_L_on_target_flag = false;
  drive_ramp_R_on_target_flag = false;
  while (!(drive_ramp_L_on_target_flag && drive_ramp_R_on_target_flag)) {
    if (drive_ramp_flag) {
      driveSetPWMRamp(max_pwm, max_pwm);
    } else {
      driveSetPWM(max_pwm, max_pwm);
      drive_ramp_L_on_target_flag = true;
      drive_ramp_R_on_target_flag = true;
    }
    vex::this_thread::sleep_for(10);
  }

  // Reduce speed once close to target?

  // Turn on PID control when close enough to target, turn off after timeout elapsed
  timeout_timer.reset();
  extra_timeout_timer.reset();
  while(drive_distance_pid_flag) {
    double current_distance = driveGetDistance();

    // PID control
    controlPID_calculation(&distance_pid, current_distance);
    driveSetPWM(distance_pid.output_pwm, distance_pid.output_pwm);

    // Reset timeout if too far from target
    if (abs(distance_mm - current_distance) > DRIVE_PID_DISTANCE_SETTLING_RANGE_MM) {
      timeout_timer.reset();
    }

    // Break out of loop after timeout elapses
    if (timeout_timer.time() > timeout_ms) {
      drive_distance_pid_flag = false;
    }

    // Break out of loop if this is taking way too long (10s)
    if (extra_timeout_timer.time() > 10000) {
      drive_distance_pid_flag = false;
    }

    vex::this_thread::sleep_for(10);
  }

  // Reset controller state, ready for next move
  driveSetPWM(0, 0);
  controlPID_resetStates(&distance_pid);
}

void driveTurnAngle(double angle_deg, int32_t max_pwm, uint32_t timeout_ms) {
  // Setup
  driveResetHeading();
  drive_distance_pid_flag = false;
  drive_rotation_pid_flag = true;
  vex::timer timeout_timer = vex::timer();
  vex::timer extra_timeout_timer = vex::timer();  // Hack for moves that don't reach settling zone
  rotation_pid.target_value = angle_deg;
  if (max_pwm > 0) {
    controlPID_setOutputRange(&rotation_pid, -max_pwm, max_pwm);
  } else {
    controlPID_setOutputRange(&rotation_pid, max_pwm, -max_pwm);
  }

  // Start move, Wait until up to full speed
  drive_ramp_L_on_target_flag = false;
  drive_ramp_R_on_target_flag = false;
  while (!(drive_ramp_L_on_target_flag && drive_ramp_R_on_target_flag)) {
    if (drive_ramp_flag) {
      driveSetPWMRamp(max_pwm, -max_pwm);
    } else {
      
      drive_ramp_L_on_target_flag = true;
      drive_ramp_R_on_target_flag = true;
    }
    vex::this_thread::sleep_for(10);
  }

  // Turn on PID control when close enough to target, turn off after timeout elapsed
  timeout_timer.reset();
  extra_timeout_timer.reset();
  while(drive_rotation_pid_flag) {
    double current_heading = driveGetHeading();

    // PID control
    controlPID_calculation(&rotation_pid, current_heading);
    driveSetPWM(rotation_pid.output_pwm, -rotation_pid.output_pwm);

    // Reset timeout if too far from target
    if (abs(angle_deg - current_heading) > DRIVE_PID_ROTATION_SETTLING_ANGLE_DEG) {
      timeout_timer.reset();
    }

    // Break out of loop after timeout elapses
    if (timeout_timer.time() > timeout_ms) {
      drive_rotation_pid_flag = false;
    }

    // Break out of loop if this is taking too long (10s)
    if (extra_timeout_timer.time() > 10000) {
      drive_rotation_pid_flag = false;
    }

    vex::this_thread::sleep_for(10);
  }

  // Reset controller state, ready for next move
  driveSetPWM(0, 0);
  controlPID_resetStates(&rotation_pid);
}