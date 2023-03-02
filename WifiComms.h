#ifndef Wifi_Comms_h
#define Wifi_Comms_h

#include <stdint.h>
#include <RingBufCPP.h>
#include "config.h"

#define NUM_PEERS 2
#define MAC_SIZE  6

// The return values aren't really defined anywhere in the 2016 API.  Just... 0 if good, bad any other value.
// https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
#define ESP_OK 0
#define MT_TX_STATUS_OK 0

#define SEND_BUFFER_SIZE 30
#define ERROR_BUFFER_SIZE 30

enum wifi_event_data_t {
  WIFI_NO_DATA = -1,
  PICKED_UP = 1,
  STARTED_SPECIAL,
  ENDED_SPECIAL,
  HEARTBEAT = 10,
};

struct WifiEvent
{
  int index;
  uint8_t mac[6];
  wifi_event_data_t signal;
};

enum wifi_error_method_t {
  UNKNOWN = -1,
  SEND_UNSENT = 1,  // Attempt to send failed completely
  SEND_FAILED,  // Got a failure status after attempt, via callback
  RCV,
  RCV_BAD_SIZE,
  NO_ERROR = 9000,
};

struct WifiFailure
{
  int index;
  uint8_t mac[6];
  wifi_error_method_t type;
};

class WifiComms {
public:
  bool begin(void);
  void update(void);
  bool sendData(const wifi_event_data_t data);
  WifiEvent getData(void);
  WifiFailure getError(void);
  void getSelfMac(uint8_t* macAdddress);
private:
  void OnSendCb(uint8_t *mac_addr, uint8_t status);
  void OnRcvCb(uint8_t* macAddress, uint8_t* data, uint8_t len);

  RingBufCPP<WifiEvent, SEND_BUFFER_SIZE> inBuffer;
  RingBufCPP<WifiFailure, ERROR_BUFFER_SIZE> errBuffer;
  volatile unsigned int signalIndex = 0;
  volatile unsigned int errorIndex = 0;

  uint8_t peerMacs[NUM_PEERS][MAC_SIZE];
  void printPeers(void);
  bool connectPeers(void);
  void printMac(const uint8_t* macAddress);
  bool isMyMacAddress(const uint8_t* macAddress);
};

#endif // Wifi_Comms_h
