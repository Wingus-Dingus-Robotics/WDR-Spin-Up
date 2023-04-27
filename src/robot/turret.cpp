#include "robot/turret.h"
#include "wdr.h"
#include "opcontrol.h"    // state_string_aimbot

// Devices
static VEX_DEVICE_GET(motor_turret, port_to_index( PORT_TURRET ));
static VEX_DEVICE_GET(motor_roller, port_to_index( PORT_ROLLER ));
static VEX_DEVICE_GET(adi_device, port_to_index( PORT_ADI ));
static uint32_t turret_pos = port_to_index( ADI_TURRET_POS );
static VEX_DEVICE_GET(adix_lower_device, port_to_index( PORT_ADIX_LOWER ));
static uint32_t solenoid_brake = port_to_index( ADIX_LOWER_TURRET_BRAKE );

// Angle control
static PID_Controller_t turret_pid;
static bool turret_is_settled = false;
static vex::timer settling_timer = vex::timer();

extern bool state_string_aimbot;

void turretInit(void) {
  // Motor
  wdr_motor_settings_t turret_settings = {
    .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
    .brake_mode = kV5MotorBrakeModeCoast,
    .encoder_unit = kMotorEncoderCounts,
    .gearset = kMotorGearSet_06,
    .reverse = false
  };
  wdrMotorInit(motor_turret, turret_settings);

  wdr_motor_settings_t roller_settings = {
    .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
    .brake_mode = kV5MotorBrakeModeBrake,
    .encoder_unit = kMotorEncoderCounts,
    .gearset = kMotorGearSet_06,
    .reverse = true
  };
  wdrMotorInit(motor_roller, roller_settings);

  // Potentiometer
  vexDeviceAdiPortConfigSet(adi_device, turret_pos, kAdiPortTypeAnalogIn);

  // Pneumatics
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_brake, kAdiPortTypeDigitalOut);
  turretBrake(false);

  // PID controller
  controlPID_init(&turret_pid, TURRET_PID_KP, TURRET_PID_KI, TURRET_PID_KD, TURRET_PID_WINDUP, TURRET_PID_DT);
  controlPID_setOutputRange(&turret_pid, -TURRET_MAX_PWM, TURRET_MAX_PWM);
  controlPID_resetStates(&turret_pid);
}

void turretPeriodic() {
  /* PID angle control */

  int32_t turret_pwm;
  controlPID_calculation(&turret_pid, turretGetAngle());
  turret_pwm = turret_pid.output_pwm;

  // Check if turret is settled
  if (abs(turret_pid.error) < TURRET_SETTLING_THRESHOLD_DEG) {
    if (settling_timer.time() > TURRET_SETTLING_TIME_MS) {
      turret_is_settled = true;
    }
  } else {
    turret_is_settled = false;
    settling_timer.reset();
  }

  // Brakes off when aimbotting
  if (state_string_aimbot || state_aimbot) {
    turret_is_settled = false;
  }

  // Brakes
  if (turret_is_settled) {
    turretBrake(true);
    turret_pwm = 0;
  } else {
    turretBrake(false);
  }

  turretSpinPWM(turret_pwm);

  /* Current limiting */

  // When brake is applied, set turrent current limit low.
  // Roller current limit is set by turretRollerSpinPWM()
  if (turret_is_settled) {
    vexMotorCurrentLimitSet(port_to_index( PORT_TURRET ), 100);
    // vexMotorCurrentLimitSet(port_to_index( PORT_ROLLER ), 2500);
  } else {
    vexMotorCurrentLimitSet(port_to_index( PORT_TURRET ), 2500);
    // vexMotorCurrentLimitSet(port_to_index( PORT_ROLLER ), 100);
  }
}

void turretDisable() {
  turretSpinPWM(0);
  turretRollerSpinPWM(0);
  turretBrake(false);
  controlPID_resetStates(&turret_pid);
}

void turretBrake(bool brake_on) {
  if (brake_on) {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_brake, 1);
  } else {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_brake, 0);
  }
}

void turretSpinPWM(int32_t pwm_value) {
  if (pwm_value > 127)       pwm_value = 127;
  else if (pwm_value < -127) pwm_value = -127;
  wdrMotorSetPwm(motor_turret, pwm_value);
}

int32_t turretGetRawReading(void) {
  return vexDeviceAdiValueGet(adi_device, turret_pos);
}

double turretGetAngle(void) {
  int32_t origin;
  double countPerDeg;
  if (misc_jumper_id == 0) {
    origin = (TURRET_LEFT_READING_ID0 + TURRET_RIGHT_READING_ID0) / 2;
    countPerDeg = (TURRET_LEFT_READING_ID0 - TURRET_RIGHT_READING_ID0) / 180.0;
  } else {
    origin = (TURRET_LEFT_READING_ID1 + TURRET_RIGHT_READING_ID1) / 2;
    countPerDeg = (TURRET_LEFT_READING_ID1 - TURRET_RIGHT_READING_ID1) / 180.0;
  }
  return -(turretGetRawReading() - origin) / countPerDeg;
}

void turretSetAngle(double angle_deg) {
  turret_pid.target_value = angle_deg;
}

double turretGetTargetAngle() {
  return turret_pid.target_value;
}

void turretRollerSpinPWM(int32_t pwm_value) {
  if (pwm_value > 127)       pwm_value = 127;
  else if (pwm_value < -127) pwm_value = -127;
  wdrMotorSetPwm(motor_roller, pwm_value);

  if (pwm_value == 0) {
    vexMotorCurrentLimitSet(port_to_index( PORT_ROLLER ), 100);
  } else {
    vexMotorCurrentLimitSet(port_to_index( PORT_ROLLER ), 2500);
  }
}