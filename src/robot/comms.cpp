#include "robot/comms.h"
#include "wdr.h"

using namespace vex;

VEX_DEVICE_GET(testSerialOut, port_to_index(PORT_SBF_TX));
VEX_DEVICE_GET(testSerialIn, port_to_index(PORT_SBF_RX));

void commsInit(void) {
  // Configure serial ports to 250kbps. Using 2 ports for full duplex operation.
  // Problems @ 1000000bps data rate. Seems okay at 500000 so drop down to
  // 250000bps for safety
  wdrSerialPortInit(testSerialOut, 250000);
  wdrSerialPortInit(testSerialIn, 250000);
}

int32_t commsUpdate(V5Cmd_t command) {
  int32_t retVal = 0;

  wdrTransmitSerialCmd(testSerialOut, command);
  retVal = wdrReceiveSerialPacket(testSerialIn, command);

  return retVal;
}
