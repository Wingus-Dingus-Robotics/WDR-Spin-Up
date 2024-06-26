#include "robot/intake.h"
#include "wdr.h"

// Intake state (used for dynamic current limiting)
bool intake_off = false;

// Intake staging states
bool intake_staging_pause_motors = false;
wdr_timer_t timer_intake_staging;

// Intake deploy
bool intake_deploy_flag = false;
bool intake_deploy_pause_motors = false;
wdr_timer_t intake_deploy_timer;


// Turret loading sequence
bool intake_loading_flag = false;
vex::timer intake_loading_timer = vex::timer();
// Separate method for loading sequence
// i.e. keep intakeTurretLoad(), add intakeTurretLoadSequence()

// Devices
static VEX_DEVICE_GET(motor_intake_L, port_to_index( PORT_INTAKE_L ));
static VEX_DEVICE_GET(motor_intake_R, port_to_index( PORT_INTAKE_R ));
static VEX_DEVICE_GET(adi_device, port_to_index( PORT_ADI ));
static uint32_t disc_detector_1 = port_to_index( ADI_DISC_1 );
static uint32_t disc_detector_2 = port_to_index( ADI_DISC_2 );
static uint32_t disc_detector_3 = port_to_index( ADI_DISC_3 );
static uint32_t disc_detector_staging = port_to_index( ADI_DISC_STAGING );
static uint32_t detectors[4] = {disc_detector_1, disc_detector_2, disc_detector_3, disc_detector_staging};
static VEX_DEVICE_GET(adix_lower_device, port_to_index( PORT_ADIX_LOWER ));
static uint32_t solenoid_deploy = port_to_index( ADIX_LOWER_INTAKE_DEPLOY );
static uint32_t solenoid_matchload = port_to_index( ADIX_LOWER_INTAKE_MATCH_LOAD );
static uint32_t solenoid_lifter = port_to_index( ADIX_LOWER_INTAKE_LIFTER );
static uint32_t solenoid_flap_L = port_to_index( ADIX_LOWER_INTAKE_FLAP_L );
static uint32_t solenoid_flap_R = port_to_index( ADIX_LOWER_INTAKE_FLAP_R );

void intakeInit(void) {
  // Motors
  wdr_motor_settings_t intake_settings = {
    .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
    .brake_mode = kV5MotorBrakeModeCoast,
    .encoder_unit = kMotorEncoderCounts,
    .gearset = kMotorGearSet_06,
    .reverse = false
  };
  wdrMotorInit(motor_intake_R, intake_settings);
  intake_settings.reverse = true;
  wdrMotorInit(motor_intake_L, intake_settings);

  // Pneumatic solenoids
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_deploy, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_matchload, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_lifter, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_flap_L, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_lower_device, solenoid_flap_R, kAdiPortTypeDigitalOut);

  // Sensors (disc detectors)
  vexDeviceAdiPortConfigSet(adi_device, disc_detector_1, kAdiPortTypeAnalogIn);
  vexDeviceAdiPortConfigSet(adi_device, disc_detector_2, kAdiPortTypeAnalogIn);
  vexDeviceAdiPortConfigSet(adi_device, disc_detector_3, kAdiPortTypeAnalogIn);
  vexDeviceAdiPortConfigSet(adi_device, disc_detector_staging, kAdiPortTypeAnalogIn);

  // Initial settings for solenoids
  intakeDeploy(false);
  intakeMatchLoad(false);
  intakeTurretLoad(false);
  intakeTurretFlaps(false);

  // States and timers
  wdrTimerInit(&timer_intake_staging);
  intake_staging_pause_motors = false;
  intake_deploy_flag = false;
  intake_deploy_pause_motors = false;
  wdrTimerInit(&intake_deploy_timer);
  intake_loading_flag = false;
}

void intakePeriodic() {
  /* Intake disc staging */ 

  if (intakeDiscDetected(4)) {
    wdrTimerReset(&timer_intake_staging);
    wdrTimerStart(&timer_intake_staging);
  }

  // bool intake_staging_pause_motors is checked by intakeSpin()

  if (wdrTimerGetTime(&timer_intake_staging) < 50) {
    // 1: Spin for 50 ms
    intake_staging_pause_motors = false;
  } else if (wdrTimerGetTime(&timer_intake_staging) < 50+100) {
    // 2: Pause for 100 ms
    intake_staging_pause_motors = true;
    intakeSpin(0);  // Make sure intake stops spinning
  } else {
    // End of behaviour
    intake_staging_pause_motors = false;
  }

  /* Intake deploy sequence */
  // Push deploy down with match loader
  if (intake_deploy_flag && (wdrTimerGetTime(&intake_deploy_timer) < 250)) {
    intakeMatchLoad(true);
    // Also prevent intake from spinning while not deployed
    intake_deploy_pause_motors = true;
  } else if (intake_deploy_pause_motors) {
    intakeMatchLoad(false);
    intake_deploy_pause_motors = false;
  }

  /* Intake turret loading sequence */
  if (intake_loading_flag) {
    if (intake_loading_timer.time() < 750) {
      // 1. Load discs into turret
      intakeTurretLoad(true);
    } else {
      // 2. Pull lifter back down
      intakeTurretLoad(false);
      if (intake_loading_timer.time() > 750+250) {
        intake_loading_flag = false;
      }
    }
  }

  /* Current limiting */

  // Intake not spinning, set current limit low
  if (intake_off) {
    vexMotorCurrentLimitSet(port_to_index( PORT_INTAKE_L ), 100);
    vexMotorCurrentLimitSet(port_to_index( PORT_INTAKE_R ), 100);
  } else {
    vexMotorCurrentLimitSet(port_to_index( PORT_INTAKE_L ), 2500);
    vexMotorCurrentLimitSet(port_to_index( PORT_INTAKE_R ), 2500);
  }
}

void intakeDisable() {
  intakeSpin(0);
}

void intakeSpin(int32_t pwm_value) {
  if (intake_staging_pause_motors) {
    pwm_value = 0;
  }

  if (intake_deploy_pause_motors) {
    pwm_value = 0;
  }

  if (pwm_value == 0) {
    intake_off = true;
  } else {
    intake_off = false;
  }
  wdrMotorSetPwm(motor_intake_L, pwm_value);
  wdrMotorSetPwm(motor_intake_R, pwm_value);
}

void intakeDeploy(bool deploy) {
  intake_deploy_flag = deploy;
  if (deploy) {
    wdrTimerStart(&intake_deploy_timer);
    vexDeviceAdiValueSet(adix_lower_device, solenoid_deploy, 1);
  } else {
    wdrTimerReset(&intake_deploy_timer);
    vexDeviceAdiValueSet(adix_lower_device, solenoid_deploy, 0);
  }
}

void intakeMatchLoad(bool load) {
  if (load) {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_matchload, 1);
  } else {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_matchload, 0);
  }
}

void intakeTurretLoad(bool load_turret) {
  if (load_turret) {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_lifter, 1);
  } else {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_lifter, 0);
  }
}

void intakeTurretLoadSequence() {
  // If not currently doing load sequence, do load sequence
  if (!intake_loading_flag) {
    intake_loading_flag = true;
    intake_loading_timer.reset();
  }

  // Actual sequence is handled in intakePeriodic
  // intakeTurretLoad() can still be used for manual control
}

void intakeTurretFlaps(bool hold) {
  if (hold) {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_flap_L, 1);
    vexDeviceAdiValueSet(adix_lower_device, solenoid_flap_R, 1);
  } else {
    vexDeviceAdiValueSet(adix_lower_device, solenoid_flap_L, 0);
    vexDeviceAdiValueSet(adix_lower_device, solenoid_flap_R, 0);
  }
}

/**
 * @brief Read value of disc detector (reflected IR light)
 * 
 * @param detector_number Level of disc inside robot (1 is lowest, 3 is highest)
 * @return int32_t Sensor value
 */
int32_t intakeGetDetectorReading(uint8_t detector_number) {
  return vexDeviceAdiValueGet(adi_device, detectors[ detector_number - 1 ]);
}

/**
 * @brief Detect if a disc is detected inside the robot
 * 
 * @param detector_number Level of disc inside robot (1 is lowest, 3 is highest)
 * @return true Disc detected
 * @return false Disc not detected
 */
bool intakeDiscDetected(uint8_t detector_number) {
  // Get threshold value
  int32_t threshold;
  switch (detector_number) {
    case 1:
      threshold = THRESHOLD_DISC_1;
      break;
    case 2:
      threshold = THRESHOLD_DISC_2;
      break;
    case 3:
      threshold = THRESHOLD_DISC_3;
      break;
    case 4:
      threshold = THRESHOLD_DISC_STAGING;
      break;
    default:
      return false;
      break;
  }

  // Detect disc
  if (intakeGetDetectorReading(detector_number) < threshold) {
    return true;
  }
  else {
    return false;
  }
}

/**
 * @brief Count the number of discs collected
 * 
 * Note that if a lower disc is missing, the function will stop counting discs any higher.
 *
 * @return uint8_t Number of discs
 */
uint8_t intakeCountDiscs() {
  uint8_t count = 0;
  if (intakeDiscDetected(1)) {
    count++;
    if (intakeDiscDetected(2)) {
      count++;
      if (intakeDiscDetected(3)) {
        count++;
      }
    }
  }
  return count;
}