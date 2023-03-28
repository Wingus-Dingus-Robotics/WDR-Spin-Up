#include "wdr/motor.h"
#include "v5_api.h"

// Initialization

// wdr_motor_settings_t wdr_default_motor_settings = {
//   .control_mode = kMotorControlModeOFF,   // V5 Motor smart controller is off.
//   .brake_mode = kV5MotorBrakeModeCoast,
//   .encoder_unit = kMotorEncoderCounts,    // Raw value pls
//   .gearset = kMotorGearSet_06,            // Doesn't really matter for us.
//   .reverse = false
// };

/**
 * @brief Initialize a V5_DeviceT as a motor
 * 
 * @param motor     Motor device to be initialized
 * @param settings  Struct of motor settings (control mode, brake mode, encoder unit, gearset)
 */
void wdrMotorInit(V5_DeviceT motor, wdr_motor_settings_t settings)
{
  vexDeviceMotorModeSet(motor, settings.control_mode);
  vexDeviceMotorBrakeModeSet(motor, settings.brake_mode);
  vexDeviceMotorEncoderUnitsSet(motor, settings.encoder_unit);
  vexDeviceMotorGearingSet(motor, settings.gearset);
  vexDeviceMotorReverseFlagSet(motor, settings.reverse);
}

// Telemetry functions

/**
 * @brief Get motor encoder position by the encoder unit (V5MotorEncoderUnits) that was set by wdrMotorInit
 * 
 * Units include: kMotorEncoderDegrees, kMotorEncoderRotations, kMotorEncoderCounts
 *
 * @param motor 
 * @return int32_t Position in unit set by wdrMotorInit
 */
int32_t wdrMotorGetPosition(V5_DeviceT motor)
{
  return vexDeviceMotorPositionGet(motor);
}

/**
 * @brief Get raw motor encoder count value
 * 
 * Side note: In v5_apitypes.h, V5_MOTOR_COUNTS_PER_ROT is defined as 1800.0 for a 36:1 gearset.
 * 
 * @param motor     Motor device
 * @param timestamp Pointer that will be updated with a timestamp of when the encoder reading occurred.
 * @return int32_t  Raw encoder value in counts
 */
int32_t wdrMotorGetRawEncoder(V5_DeviceT motor, uint32_t *timestamp)
{
  return vexDeviceMotorPositionRawGet(motor, timestamp);
}

/**
 * @brief Get motor current in [mA]
 * 
 * @param motor     Motor device
 * @return int32_t  Current draw in [mA]
 */
int32_t wdrMotorGetCurrent(V5_DeviceT motor)
{
  return vexDeviceMotorCurrentGet(motor);
}

/**
 * @brief Get motor voltage in [mV]
 * 
 * @param motor     Motor device
 * @return int32_t  Voltage in [mV]
 */
int32_t wdrMotorGetVoltage(V5_DeviceT motor)
{
  return vexDeviceMotorVoltageGet(motor);
}

/**
 * @brief Get motor temperature in [Celcius]
 * 
 * @param motor     Motor device
 * @return double   Temperature in [Celcius]
 */
double wdrMotorGetTemperature(V5_DeviceT motor)
{
  return vexDeviceMotorTemperatureGet(motor);
}

/**
 * @brief Get motor velocity in [rpm]
 * 
 * Is reportedly a bit shit: https://sylvie.fyi/sylib/docs/db/d8e/md_module_writeups__velocity__estimation.html
 * My guess is that it just takes two timestamped raw encoder readings. (d2 - d1) /  (t2 - t1)
 *
 * @param motor     Motor device
 * @return double   Velocity in [rpm]
 */
double wdrMotorGetVelocity(V5_DeviceT motor)
{
  // In V5 API, "Actual Velocity" is different to the "Velocity setpoint" (vexDeviceMotorVelocityGet).
  return vexDeviceMotorActualVelocityGet(motor);
}

/**
 * @brief Get motor torque in [Nm]
 * 
 * @param motor     Motor device
 * @return double   Torque in [Nm]
 */
double wdrMotorGetTorque(V5_DeviceT motor)
{
  return vexDeviceMotorTorqueGet(motor);
}

/**
 * @brief Get all telemetry data available for motor
 * 
 * @param motor                   Motor device
 * @return wdr_motor_telemetry_t  Struct containing telemetry data at a particular timestamp
 */
wdr_motor_telemetry_t wdrMotorGetAllTelemetry(V5_DeviceT motor)
{
  wdr_motor_telemetry_t telemetry;
  telemetry.encoder_counts = wdrMotorGetRawEncoder(motor, &telemetry.timestamp_ms);
  telemetry.current_mA = wdrMotorGetCurrent(motor);
  telemetry.voltage_mV = wdrMotorGetVoltage(motor);
  telemetry.temperature_C = wdrMotorGetTemperature(motor);
  // telemetry.velocity_rpm = wdrMotorGetVelocity(motor);
  // telemetry.torque_Nm = wdrMotorGetTorque(motor);
  return telemetry;
}

// Movement functions

/**
 * @brief Move motor by setting PWM duty cycle
 * 
 * @param motor     Motor device
 * @param pwm_value PWM duty from -127 to 127 I think?
 */
void wdrMotorSetPwm(V5_DeviceT motor, int32_t pwm_value)
{
  // Clip min/max value
  if (pwm_value > MOTOR_PWM_MAX)  pwm_value = MOTOR_PWM_MAX;
  else if (pwm_value < -MOTOR_PWM_MAX)  pwm_value = -MOTOR_PWM_MAX;
  vexDeviceMotorPwmSet(motor, pwm_value);
}

/**
 * @brief Move motor by setting voltage
 * 
 * @param motor         Motor device
 * @param voltage_value Voltage in [mV] from -1200 to 1200
 */
void wdrMotorSetVoltage(V5_DeviceT motor, int32_t voltage_value)
{
  // Clip min/max value
  if (voltage_value > MOTOR_VOLTAGE_MAX)  voltage_value = MOTOR_PWM_MAX;
  else if (voltage_value < -MOTOR_VOLTAGE_MAX)  voltage_value = -MOTOR_VOLTAGE_MAX;
  vexDeviceMotorVoltageSet(motor, voltage_value);
}

// Configuration functions

void wdrMotorSetCurrentLimit(V5_DeviceT motor, int32_t value)
{
  vexDeviceMotorCurrentLimitSet(motor, value);
}

void wdrMotorSetVoltageLimit(V5_DeviceT motor, int32_t value)
{
  vexDeviceMotorVoltageSet(motor, value);
}

void wdrMotorSettingControlMode(V5_DeviceT motor, V5MotorControlMode mode)
{
  vexDeviceMotorModeSet(motor, mode);
}

void wdrMotorSettingBrakeMode(V5_DeviceT motor, V5MotorBrakeMode mode)
{
  vexDeviceMotorBrakeModeSet(motor, mode);
}

void wdrMotorSettingDirection(V5_DeviceT motor, bool reverse_dir)
{
  vexDeviceMotorReverseFlagSet(motor, reverse_dir);
}

// Controllers
//e.g. Ramping