/**
 * @file motor.h
 * @author your name (you@domain.com)
 * @brief Functions for interacting with V5 Smart Motors.
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef MOTOR_H
#define MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

// #include <stdint.h>
// #include <stdbool.h>
#include "v5_apitypes.h"

#define MOTOR_PWM_MAX       127
#define MOTOR_VOLTAGE_MAX   1200

// Initialization
typedef struct {
  V5MotorControlMode control_mode;
  V5MotorBrakeMode brake_mode;
  V5MotorEncoderUnits encoder_unit;
  V5MotorGearset gearset;
  bool reverse;                      //Should this be a setting?
} wdr_motor_settings_t;   // alternate naming: wdr_MotorSettingsT
void wdrMotorInit(V5_DeviceT motor, wdr_motor_settings_t settings);

// Telemetry
int32_t wdrMotorGetPosition(V5_DeviceT motor);
int32_t wdrMotorGetRawEncoder(V5_DeviceT motor, uint32_t *timestamp);
int32_t wdrMotorGetCurrent(V5_DeviceT motor);
int32_t wdrMotorGetVoltage(V5_DeviceT motor);
double wdrMotorGetTemperature(V5_DeviceT motor);

double wdrMotorGetVelocity(V5_DeviceT motor);
double wdrMotorGetTorque(V5_DeviceT motor);

typedef struct wdr_motor_telemetry
{
  // Single (raw?) measurement
  uint32_t timestamp_ms;
  int32_t encoder_counts;
  int32_t current_mA;
  int32_t voltage_mV;
  double temperature_C;

  // // Calculated over unspecified time period (probably just 2 timestamped encoder position values)
  // double velocity_rpm;    // We can calculate this on our own too, using timestamped encoder counts.
  // double torque_Nm;       // We can calculate this on our own too, using voltage, velocity, and estimating motor constant and stator resistance.
} wdr_motor_telemetry_t;
wdr_motor_telemetry_t wdrMotorGetAllTelemetry(V5_DeviceT motor);
//TODO: Get Flags? (e.g. over temperature, over current)

// Movement
void wdrMotorSetPwm(V5_DeviceT motor, int32_t pwm_value);
void wdrMotorSetVoltage(V5_DeviceT motor, int32_t voltage_value);       //TODO: Will this adjust PWM based on current battery level? Is value in volts? Probably not, needs finer control than that.
// void wdrMotorSetPercent(double percent_value);

// Set Limits
void wdrMotorSetCurrentLimit(V5_DeviceT motor, int32_t value);
void wdrMotorSetVoltageLimit(V5_DeviceT motor, int32_t value);

// Set Motor settings
void wdrMotorSettingControlMode(V5_DeviceT motor, V5MotorControlMode mode);
void wdrMotorSettingBrakeMode(V5_DeviceT motor, V5MotorBrakeMode mode);
void wdrMotorSettingDirection(V5_DeviceT motor, bool reverse_dir);

//TODO: Position/Velocity controller. How do you change PID values?

//TODO: Custom ramping/slew rate controller? Make a "wdrMotorControlUpdate" function or something.

#ifdef __cplusplus
}
#endif
#endif // MOTOR_H
