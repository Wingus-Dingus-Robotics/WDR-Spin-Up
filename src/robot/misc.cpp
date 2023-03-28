#include "robot/misc.h"
#include "wdr.h"

// Devices
static VEX_DEVICE_GET(adi_device, port_to_index( PORT_ADI ));
static uint32_t solenoid_string = port_to_index( ADI_END_GAME );
static uint32_t auto_select = port_to_index( ADI_AUTO_SELECT );
static uint32_t id_jumper = port_to_index( ADI_JUMPER_ID );

bool misc_jumper_id;

void miscInit() {
  vexDeviceAdiPortConfigSet(adi_device, auto_select, kAdiPortTypeAnalogIn);
  vexDeviceAdiPortConfigSet(adi_device, solenoid_string, kAdiPortTypeDigitalOut);
  vexDeviceAdiPortConfigSet(adi_device, id_jumper, kAdiPortTypeDigitalIn);

  miscString(false);
  misc_jumper_id = miscGetJumperID();
}

void miscString(bool shoot) {
  if (shoot) {
    vexDeviceAdiValueSet(adi_device, solenoid_string, 1);
  } else {
    vexDeviceAdiValueSet(adi_device, solenoid_string, 0);
  }
}

uint32_t miscGetAutoSelect() {
  return vexDeviceAdiValueGet(adi_device, auto_select);
}

bool miscGetJumperID() {
  if (vexDeviceAdiValueGet(adi_device, id_jumper) == 0) {
    return false;
  } else {
    return true;
  }
}