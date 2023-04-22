#include "wdr/serial.h"
#include "v5_api.h"
#include "v5_cpp.h"

using namespace vex;

// Global variables (initial definition)
sbf_data_t sbf_data = {0};  // NOW UNSED.
sbf_pose_t sbf_pose = {0};

/**
 * @brief Initialize V5 Smart Port for serial operations
 *
 * @param SerialPort V5 Smart Port to use as serial port
 * @param baudrate Baud rate in bps. Use 250000 bps if unsure.
 *
 * Problems @ 1000000bps data rate. Seems okay at 500000 so drop down to
 * 250000bps for safety
 */
void wdrSerialPortInit(V5_DeviceT SerialPort, int32_t baudrate) {
  vexDeviceGenericSerialEnable(SerialPort, 0);
  vexDeviceGenericSerialBaudrate(SerialPort, baudrate);
  vexDeviceGenericSerialFlush(SerialPort);
}

/**
 * @brief Transmit command to SBF board
 *
 * @param SerialOut V5 Smart Port configured as serial port
 * @param v5Cmd Command char to send to SBF
 */
void wdrTransmitSerialCmd(V5_DeviceT SerialOut, V5Cmd_t v5Cmd) {
  vexDeviceGenericSerialWriteChar(SerialOut, v5Cmd);
}

/**
 * @brief 
 * 
 * @param SerialIn V5 Smart Port configured as serial port
 * @param command UNUSED
 * @return int32_t Returns an SBFErrorCode if error occurred, otherwise 0
 */
int32_t wdrReceiveSerialPacket(V5_DeviceT SerialIn, uint8_t command) {
  const uint8_t MAX_WAIT = 50;
  uint8_t packetStartWait = 0;
  int32_t ch = 0;
  int32_t v5CmdResponse = 0; // v5Cmd
  int32_t packetLength = 0;
  int32_t rxByteCount = 0;
  uint8_t inBuffer[128];

  while ((vexDeviceGenericSerialPeekChar(SerialIn) == -1) &&
         (packetStartWait < MAX_WAIT)) {
    this_thread::sleep_for(1);
    packetStartWait++;
  }
  // Something went wrong with receive. Maybe not plugged in?
  if (packetStartWait >= MAX_WAIT) {
    return SBFErrorCode::TIMEOUT;
  }

  // Check for packet header.
  ch = vexDeviceGenericSerialReadChar(SerialIn);
  if (ch != 0x7E) {
    vexDeviceGenericSerialFlush(SerialIn);
    return SBFErrorCode::INVALID_HEADER;
  }

  // Get original command sent from V5 to SBF
  v5CmdResponse = vexDeviceGenericSerialReadChar(SerialIn);
  if (v5CmdResponse == -1) {
    vexDeviceGenericSerialFlush(SerialIn);
    return SBFErrorCode::INVALID_COMMAND;   // TODO: More extensive command check?
  }

  // Check packet length
  packetLength = vexDeviceGenericSerialReadChar(SerialIn);
  // **FixMe Check for valid length
  if (packetLength == -1) {
    vexDeviceGenericSerialFlush(SerialIn);
    return SBFErrorCode::INVALID_LENGTH;
  }

  // Get data
  rxByteCount =
      vexDeviceGenericSerialReceive(SerialIn, inBuffer, packetLength - 1);
  if (rxByteCount < packetLength - 1) {
    vexDeviceGenericSerialFlush(SerialIn);
    return SBFErrorCode::INVALID_DATA;  // Data too short
  }

  // Checksum8
  uint8_t loop = 0;
  uint8_t checkSum8 = 0;

  checkSum8 = (uint8_t)v5CmdResponse + (uint8_t)packetLength;
  for (loop = 0; loop < packetLength - 2; loop++) {
    checkSum8 += inBuffer[loop];
  }
  if (inBuffer[packetLength - 2] != checkSum8) {
    return SBFErrorCode::FAIL_CHECKSUM;
  }

  // Update global variable with new data
  switch ((uint8_t)v5CmdResponse) {
  case V5Cmd_t::ENC1: {
    uint32_t *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[0];
    sbf_data.ENC1 = *encValPtr;
    break;
  }
  case V5Cmd_t::ENC2: {
    uint32_t *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[0];
    sbf_data.ENC2 = *encValPtr;
    break;
  }
  case V5Cmd_t::ENC3: {
    uint32_t *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[0];
    sbf_data.ENC3 = *encValPtr;
    break;
  }
  // case '4': {
  //   uint32_t *encValPtr;
  //   encValPtr = (uint32_t *)&inBuffer[0];
  //   sbf_ENC4 = *encValPtr;
  //   break;
  // }
  case V5Cmd_t::ALL: {
    uint32_t *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[0];
    sbf_data.ENC1 = *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[4];
    sbf_data.ENC2 = *encValPtr;
    encValPtr = (uint32_t *)&inBuffer[8];
    sbf_data.ENC3 = *encValPtr;
    break;
  }
  case V5Cmd_t::POSE: {
    uint32_t *poseValPtr;
    poseValPtr = (uint32_t *)&inBuffer[0];
    sbf_pose.x = *poseValPtr;
    poseValPtr = (uint32_t *)&inBuffer[4];
    sbf_pose.y = *poseValPtr;
    poseValPtr = (uint32_t *)&inBuffer[8];
    sbf_pose.theta = *poseValPtr;
  }

  default: {
  }
  }

  return 0; // Return successful receive
}
