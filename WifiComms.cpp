#include "WifiComms.h"

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <RingBuf.h>

// See also: PeerStates.h
const uint8_t allMacAddresses[3][MAC_SIZE] = {
  MAC_ADDR_1,
  MAC_ADDR_2,
  MAC_ADDR_3
};

bool WifiComms::begin() {

  WiFi.mode(WIFI_AP);  // Recommended for nodes in COMBO mode.

  WiFi.disconnect(); // What this do?
  WiFi.softAPdisconnect(false); // What this do?
  
  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.macAddress());
  Serial.print("Wifi MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());


  uint8_t myMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WiFi.softAPmacAddress(myMacAddress);


  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNow Init Failed");
    return false;
  }

  if (esp_now_set_self_role(ESP_NOW_ROLE_COMBO) != ESP_OK) {
    Serial.println("Failed to set device role.");
  }
  
  // Only try to communicate with peers that aren't ME...
  int peerNum = 0;
  for (int i = 0; i < 3 && peerNum < NUM_PEERS; i++) {
    if (!isMyMacAddress(allMacAddresses[i])) {
      // No assigning to arrays, only memcpy!
      memcpy(peerMacs[peerNum], allMacAddresses[i], MAC_SIZE);
      peerNum++;
    }
  }

  // Oh boy, C-style callbacks.  We need to register a static callback but still want access to member variables...
  // Thankfully, C++11 makes this a lot easier than it used to be.  Still really damn ugly, though.
    static auto rxCb = [this](uint8_t* mac_addr, uint8_t* data, uint8_t len) {
        // because we have a this pointer we are now able to call a non-static member method:
        OnRcvCb(mac_addr, data, len);
    };
    // Pass a static lambda here, which calls the rxCb with a "this" captured.
    esp_now_register_recv_cb([](uint8_t* mac_addr, uint8_t* data, uint8_t len) {
      rxCb(mac_addr, data, len);
    });

    static auto txCb = [this](uint8_t *mac_addr, uint8_t status) {
      // because we have a this pointer we are now able to call a non-static member method:
      OnSendCb(mac_addr, status);
    };
    // Pass a static lambda here, which calls the rxCb with a "this" captured.
    esp_now_register_send_cb([](uint8_t *mac_addr, uint8_t status) {
      txCb(mac_addr, status);
    });
  return connectPeers();
}

bool WifiComms::connectPeers(void) {
  for(int i = 0; i < NUM_PEERS; i++) {
    uint8_t* potentialPeer = peerMacs[i];
    int res = esp_now_add_peer(potentialPeer, ESP_NOW_ROLE_COMBO, 5, NULL, 0);
    switch (res) {
      case ESP_OK:
        break;
      default:
        Serial.print("Uncaught result for connectPeer: \t");
        printMac(potentialPeer);
        Serial.print("\t");  Serial.print(res); Serial.println();
        return false;
        break;
    }
  }
  return true;  
}

unsigned long lastBufferCheck = 0;
void WifiComms::update(void) {
  if (errBuffer.numElements() > ERROR_BUFFER_SIZE / 2) {
    const unsigned long now = millis();
    if (lastBufferCheck + 5000 <= now) {
      Serial.print("WifiComms: Errors are accumulating rapidly, or not being consumed (");  Serial.print(errBuffer.numElements()); Serial.println(" errors");
      lastBufferCheck = now;
    }
   }
}

// This is happening via interrupt, so keep the amount of work happening here low.
// Serial.print() will frequently cause failures of other interrupt-based behavior.
void WifiComms::OnSendCb(uint8_t *mac_addr, uint8_t status) {
  if (status != ESP_OK) {
    WifiFailure err;
    err.index = errorIndex;
    err.type = SEND_FAILED;
    memcpy(err.mac, mac_addr, 6);
    errBuffer.add(err);
    errorIndex++;
  }
}

// This is happening via interrupt, so keep the amount of work happening here low.
// Serial.print() will frequently cause failures of other interrupt-based behavior.
void WifiComms::OnRcvCb(uint8_t* mac_addr, uint8_t* data, uint8_t len) {
  const int expectedSize = sizeof(wifi_event_data_t);
  if (len != expectedSize) {
    WifiFailure err;
    err.index = errorIndex;
    err.type = RCV_BAD_SIZE;
    memcpy(err.mac, mac_addr, 6);
    errBuffer.add(err);
    errorIndex++;
    //Serial.print("Invalid sized transmission, ignoring. Expected: ");  Serial.print(expectedSize); Serial.print(" got: "); Serial.println(len);
    return;
  }

  struct WifiEvent e;
  e.index = signalIndex; // TODO: Increment?
  e.signal = (wifi_event_data_t) *data;
  memcpy(e.mac, mac_addr, 6);
  if (!inBuffer.add(e)) {
    //Serial.println("Failed to add to buffer.");
  }

  signalIndex++;
}

bool WifiComms::sendData(const wifi_event_data_t data) {

  uint8_t* nextMac;
  bool start = true;
  nextMac = esp_now_fetch_peer(start);
 
  while (nextMac) {
    if (esp_now_send(nextMac, (uint8_t*) &data, sizeof(data)) == ESP_OK) {  
    } else {
      // Serial.println("esp_now_send failed.");
    }
    nextMac = esp_now_fetch_peer(false);
    //delay(1); // This delay substantially decreased the number of failed sends during rapidfire transmissions.
  }
  return true;
}

WifiEvent WifiComms::getData(void) {
  struct WifiEvent signal;
  if (!inBuffer.pull(&signal)) {
    signal.index = WIFI_NO_DATA;
  }
  return signal;  
}

WifiFailure WifiComms::getError(void) {
  struct WifiFailure fail;
  if (!errBuffer.pull(&fail)) {
    fail.type = NO_ERROR;
  }
  return fail;
}

void WifiComms::getSelfMac(uint8_t *macAddress) {
  WiFi.softAPmacAddress(macAddress);
}

void WifiComms::printPeers(void) {
    
}

void WifiComms::printMac(const uint8_t* macAddress) {
  for (int j = 0; j < MAC_SIZE; j++) { 
    Serial.print(macAddress[j], HEX);
    if (j < (MAC_SIZE - 1)) {
      Serial.print(":");
    } 
  }
}

bool WifiComms::isMyMacAddress(const uint8_t* macAddress) {
  uint8_t myMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WiFi.softAPmacAddress(myMacAddress);

  int matches = 0;

  for (int i = 0; i < MAC_SIZE; i++) {
    if (myMacAddress[i] == macAddress[i]) {
      matches++;
    }
  }
  return matches == MAC_SIZE;
}