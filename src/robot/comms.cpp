#include "robot/comms.h"
#include "wdr.h"
#include "opcontrol.h"

using namespace vex;

VEX_DEVICE_GET(sbfSerialOut, port_to_index(PORT_SBF_TX));
VEX_DEVICE_GET(sbfSerialIn, port_to_index(PORT_SBF_RX));
VEX_DEVICE_GET(debugOut, port_to_index( PORT_DEBUG ));

void commsInit(void) {
  // Configure SBF serial ports to 250kbps. Using 2 ports for full duplex operation.
  // Problems @ 1000000bps data rate. Seems okay at 500000 so drop down to
  // 250000bps for safety
  wdrSerialPortInit(sbfSerialOut, 250000);
  wdrSerialPortInit(sbfSerialIn, 250000);
  wdrSerialPortInit(debugOut, 115200);
}

V5Cmd_t prev_LED = LED_ALIVE;
static V5Cmd_t competitionLED() {
  // No. of discs collected
  switch (intakeCountDiscs()) {
  case 1:
    return LED_GREEN;
    break;

  case 2:
    return LED_ORANGE;
    break;

  case 3:
    return LED_RED;
    break;

  default:
    if (launcherFlickCountDiscs() > 0) {
      return prev_LED;
    } else {
      return LED_ALIVE;
    }
  }
}

static V5Cmd_t competitionFlash() {
  // Flashing
  if (launcherFlickCountDiscs()) {
    return FLASH_SLOW;
  } else {
    return FLASH_STOP;
  }
}

int32_t commsUpdate(V5Cmd_t command) {
  /* Flashing lights*/
  prev_LED = competitionLED();
  if (wdrGetCompStatus() == Disabled) {
    wdrTransmitSerialCmd(sbfSerialOut, BLANK);
  } else if (state_string_aimbot) {
    wdrTransmitSerialCmd(sbfSerialOut, LED_CYAN);
  } else if (state_aimbot) {
    wdrTransmitSerialCmd(sbfSerialOut, LED_BLUE);
  } else {
    wdrTransmitSerialCmd(sbfSerialOut, prev_LED);
  }
  wdrTransmitSerialCmd(sbfSerialOut, competitionFlash());

  /* Everything else*/

  int32_t retVal = 0;

  wdrTransmitSerialCmd(sbfSerialOut, command);
  retVal = wdrReceiveSerialPacket(sbfSerialIn, command);

  // Hack
  wdrTransmitSerialCmd(sbfSerialOut, V5Cmd_t::ENC1);
  wdrReceiveSerialPacket(sbfSerialIn, V5Cmd_t::ENC1);

  // // TODO: Add debug section here
  // int32_t data = 5;
  // // 1. Convert data to csv row
  // char buff[10] = {0};
  // sprintf(buff, "%d, %d\r\n", data, 2);

  // // 2. Send each char
  // for (int i=0; i<10; i++) {
  //   vexDeviceGenericSerialWriteChar(debugOut, buff[i]);
  // }

  return retVal;
}
