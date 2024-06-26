#include "robot/misc.h"
#include "wdr.h"

// Devices
static VEX_DEVICE_GET(adi_device, port_to_index( PORT_ADI ));
static uint32_t auto_select = port_to_index( ADI_AUTO_SELECT );

static VEX_DEVICE_GET(adix_upper_device, port_to_index( PORT_ADIX_UPPER ));
static uint32_t solenoid_string_1 = port_to_index( ADIX_UPPER_STRING_1 );
static uint32_t solenoid_string_2 = port_to_index( ADIX_UPPER_STRING_2 );
static uint32_t id_jumper = port_to_index( ADIX_UPPER_JUMPER_ID );

bool misc_jumper_id;

void miscInit() {
  vexDeviceAdiPortConfigSet(adi_device, auto_select, kAdiPortTypeAnalogIn);
  vexDeviceAdiPortConfigSet(adix_upper_device, solenoid_string_1, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_upper_device, solenoid_string_2, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adix_upper_device, id_jumper, kAdiPortTypeDigitalIn);

  miscStringL(false);
  miscStringR(false);
  misc_jumper_id = miscGetJumperID();
}

void miscStringL(bool shoot) {
  // Shoot left string first.
  if (shoot) {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_string_1, 1);
  } else {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_string_1, 0);
  }
}

void miscStringR(bool shoot) {
  if (shoot) {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_string_2, 1);
  } else {
    vexDeviceAdiValueSet(adix_upper_device, solenoid_string_2, 0);
  }
}

uint32_t miscGetAutoSelect() {
  return vexDeviceAdiValueGet(adi_device, auto_select);
}

bool miscGetJumperID() {
  if (vexDeviceAdiValueGet(adix_upper_device, id_jumper) == 0) {
    return false;
  } else {
    return true;    // White/15" robot
  }
}