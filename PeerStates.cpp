#include "PeerStates.h"
#include "config.h"

// CPP knowledge failure here
PeerStates::PeerStates() :
 devices{
   {MAC_ADDR_1, false, 0 },  
   {MAC_ADDR_2, false, 0 },
   {MAC_ADDR_3, false, 0 }
 }, pickupWindow(0) 
{}

void PeerStates::begin(const uint8_t *myMacAddress, const unsigned long window) {
  pickupWindow = window;
  device_state_t firstDevice;
  uint8_t test[6];

  unsigned long now = millis();
  for (int i = 0; i < MAX_PEERS; i++) {
    devices[i].lastUpdate = now;   
  }
}

void PeerStates::update(unsigned long currentMs) {
  

  for (int i = 0; i < MAX_PEERS; i++) {
    device_state_t& device = devices[i];

    if (device.mac[0] == 0) continue;

    if (device.pickedUp && (device.lastUpdate + pickupWindow < currentMs)) {
      Serial.print("Expiring peer pickup: "); Serial.println(i);
      device.pickedUp = false;
      device.lastUpdate = currentMs;
    }
  }
}

bool PeerStates::hasPeer(const uint8_t *macAddress) {
  return findPeer(macAddress) >= 0;
}

bool PeerStates::activatePeer(const uint8_t *macAddress) {
  int idx = findPeer(macAddress);
  if (idx < 0) {
    Serial.println("Failed to find peer.");
    return false;
  }

  if (!devices[idx].pickedUp) {
    Serial.print("Activating peer ");  Serial.println(idx);
  }
  unsigned long now = millis();
  devices[idx].lastUpdate = now;
  devices[idx].pickedUp = true;

  return true;
}

bool PeerStates::activateAllPeers(void) {
  Serial.println("Setting all peers active.");
  return activatePeer(devices[0].mac) && activatePeer(devices[1].mac) && activatePeer(devices[2].mac);
}

bool PeerStates::isAllActive(void) {
  int actives = 0;
  for (int i = 0; i < MAX_PEERS; i++) {
    if (devices[i].pickedUp) {
      actives++;
    }
  }

  //Serial.print("Actives: ");  Serial.println(actives);

  return actives == MAX_PEERS;
}

int PeerStates:: findPeer(const uint8_t *macAddress) {
  const int FAILED_INDEX = -1;
  for (int i = 0; i < MAX_PEERS; i++) {
    int matches = 0;
    device_state_t device = devices[i];
    
    for (int j = 0; j < MAC_SIZE; j++) {
      if (macAddress[j] != device.mac[j]) break;
      matches++;
    }

    if (matches == MAC_SIZE) {
      return i;
    }
  }

  return FAILED_INDEX;
}