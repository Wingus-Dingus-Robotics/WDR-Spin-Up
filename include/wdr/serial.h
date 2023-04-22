#ifndef WDR_SERIAL_H
#define WDR_SERIAL_H

#include "v5_apitypes.h"

// Data structure for SBF return data (used for global variable in wdr.h)
typedef struct {
  uint32_t ENC1;
  uint32_t ENC2;
  uint32_t ENC3;
  uint32_t IMU;
} sbf_data_t;

typedef struct {
  uint32_t x;     // Tenths of mm, Right is +ve
  uint32_t y;     // Tenths of mm, Forwards is +ve
  uint32_t theta; // Tenths of degree, CW is +ve
} sbf_pose_t;

// Commands from V5 to SBF
typedef enum {
  ENC1 = '1',   // Return value of ENC1
  ENC2 = '2',   // ...
  ENC3 = '3',
  IMU = 'I',    // Return value of BNO055 IMU (TODO: What value(s)?)
  ALL = '0',    // Return every ENC value
  POSE = 'P'    // Return robot pose (x, y, theta) [mm] and [deg]
} V5Cmd_t;

enum SBFErrorCode {
  TIMEOUT = -1,
  INVALID_HEADER = -2,
  INVALID_COMMAND = -3,
  INVALID_LENGTH = -4,
  INVALID_DATA = -5,
  FAIL_CHECKSUM = -6
};

// Initialization of serial ports
void wdrSerialPortInit(V5_DeviceT SerialPort, int32_t baudrate);

// Serial data transmission requests to SBF board
void wdrTransmitSerialCmd(V5_DeviceT SerialOut, V5Cmd_t v5Cmd);

// Serial data reception. Expecting correctly formed packets from SBF
int32_t wdrReceiveSerialPacket(V5_DeviceT SerialIn, uint8_t command);

#endif // WDR_SERIAL_H
