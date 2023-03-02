#ifndef Peer_States_h
#define Peer_States_h

#include <stdint.h>
#include "WifiComms.h"

#define MAX_PEERS 3

// Define some types that can be used to track peer states we're told about.
struct device_state_t {
  uint8_t mac[MAC_SIZE] = { 0x00 };
  bool pickedUp = false;
  unsigned long lastUpdate = 0;
};

class PeerStates {
public:
  PeerStates();
  void begin(const uint8_t *myMacAddress, const unsigned long pickupWindow);
  void update(unsigned long currentMs);
  bool hasPeer(const uint8_t *macAddress);
  bool addPeer(const uint8_t *macAddress);
  bool activatePeer(const uint8_t *macAddress);
  bool activateAllPeers(void);
  bool isAllActive(void);

private:
  unsigned long pickupWindow;
  device_state_t devices[MAX_PEERS];

  int findPeer(const uint8_t *macAddress);
};

#endif // Peer_States_h
