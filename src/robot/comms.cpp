#include "robot/comms.h"
#include "wdr.h"

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

int32_t commsUpdate(V5Cmd_t command) {
  /* Flashing lights*/
  wdrTransmitSerialCmd(sbfSerialOut, LED_ALIVE);

  /* Everything else*/

  int32_t retVal = 0;

  wdrTransmitSerialCmd(sbfSerialOut, command);
  retVal = wdrReceiveSerialPacket(sbfSerialIn, command);

  // TODO: Add debug section here
  int32_t data = 5;
  // 1. Convert data to csv row
  char buff[10] = {0};
  sprintf(buff, "%d, %d\r\n", data, 2);

  // 2. Send each char
  for (int i=0; i<10; i++) {
    vexDeviceGenericSerialWriteChar(debugOut, buff[i]);
  }

  return retVal;
}
