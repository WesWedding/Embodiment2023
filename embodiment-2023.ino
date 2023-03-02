/**************************************************************************/
/*!
    @file     embodiment-2023.ino
    @author   Weston Wedding
    @license  expat (see license.md)

    This software is written as part of a contribution to the Northwest
    Arkansas Community College's 2023 Spring Arts & Cultural Festival.
*/
/**************************************************************************/

#include "config.h"

// Include stuff for our Accelerometer sensing...
#include "AccelerometerWatcher.h"

// Include the Wifi Access Point code...
#include "WifiComms.h"

// Include Sound and SD Card code...
#include "SoundPlayer.h"

// Include tracking of this and other devices' "picked up" status
#include "PeerStates.h"

AccelerometerWatcher accel(SENSE_SAMPLE_RATE, SENSE_AVERAGE_COUNT, SENSE_PICKUP_DURATION);

WifiComms wifi;

SoundPlayer player(SOUND_NORMAL_FILE, SOUND_SPECIAL_FILE);

PeerStates peers;

unsigned long lastDebugMs = 0;
unsigned long prevSendPickedUp = 0;
unsigned long prevHeartbeat = 0;  // Sending out a "heartbeet" to peers on occasion.

bool notifiedSpecial = false; // Helps prevent excessive signal transmission when all peers are activated.

void setup(void) {
  Serial.begin(115200);
 
  // Differentiate our Serial output from the messy stuff that spits out of a Feather boot.
  Serial.println();
  Serial.println("------  SETUP  ------");

  if (! player.begin()) { // initialise the sound player
    Serial.println(F("Couldn't find VS1053 (audio module), or SD card missing."));
  } else {
    Serial.println(F("VS1053 (audio module) started!"));
  }

  delay(500);
  
  if (! wifi.begin()) {
    Serial.println(F("Couldnt start wifi."));
  } else {
    Serial.println(F("Wifi started!"));
  }

  uint8_t myMac[MAC_SIZE];
  wifi.getSelfMac(myMac);
  peers.begin(myMac, SENSE_PICKUP_DURATION);

  //wifi.sendData(wifi_event_data_t::HEARTBEAT); // startup heartbeat
  prevHeartbeat = millis();

  delay(500);
  
  // The accelerometer is prone to not initialize 
  if (! accel.begin()) {
    Serial.println(F("Couldnt start MMA8451 (accelerometer)!!  Try to unplug/replug the device!"));
  } else {
    Serial.println(F("MMA8451 (accelerometer) found."));
  }

  Serial.println("--- SETUP COMPLETED ---"); Serial.println(); Serial.println();
}

void loop() {
  const unsigned long currentMs = millis();

  accel.update(currentMs);
  wifi.update();
  peers.update(currentMs);
  player.update(currentMs);

  const bool wasPickedUp = accel.wasPickedUp();
 
  // Do a heartbeat every once in a while, just for funsies.
  if (prevHeartbeat + 10000 < currentMs) {          
    wifi.sendData(wifi_event_data_t::HEARTBEAT); // heartbeat
    prevHeartbeat = currentMs;
  }

  // Handle signals and adjust PeerStates accordingly.
  struct WifiEvent newSignal = wifi.getData();
  if (newSignal.index != WIFI_NO_DATA) {
    //Serial.print("signal # ");  Serial.println(newSignal.index);
    switch (newSignal.signal) {
      case PICKED_UP:
        //Serial.println("Handling pickup signal.");
        peers.activatePeer(newSignal.mac);
        break;
      case STARTED_SPECIAL:
        Serial.println("Handling special signal.");
        // It doesn't matter if our PeerStates indicate that everyone has been activated or not at this point; we might have missed a signal.
        peers.activateAllPeers();
        break;
      case HEARTBEAT:
        //Serial.print("Heartbeat from: ");  Serial.println(newSignal.mac[5], HEX);
        break;
      default:
        Serial.print("Unrecognized signal: "); Serial.println(newSignal.signal);
        break;
    } 
  }

  if (notifiedSpecial && !peers.isAllActive()) {
    notifiedSpecial = false;
  }

  // Finally, have WE been picked up?
  if (wasPickedUp) {
    uint8_t myMac[MAC_SIZE];
    wifi.getSelfMac(myMac);
    peers.activatePeer(myMac);
  }

  // Now that peer states have been determined, what signals should we send?  What should we play?
  if (peers.isAllActive()) {

    // Unlike "picked up" messages, we don't want to excessively spam "special" signal.
    // We want it to settle down and reset once this state is reached.
    if (!notifiedSpecial) {
      Serial.println("Feeling special!");
      notifiedSpecial = true;
      wifi.sendData(wifi_event_data_t::STARTED_SPECIAL);
    }
    // We want to keep the special sound playing for as long as we're all active
    if (!player.play(sound_file_t::SPECIAL_FILE)) {
      Serial.println("Error: Failed to play special file.");
    }
  }
  
  
  // We are reading acceleration constantly, but we don't want to choke our comms with dozens of signals
  // we second.
  const unsigned long timeSinceLastPickedup = currentMs - prevSendPickedUp;
  if (wasPickedUp && timeSinceLastPickedup > 500) {
  
    // We still want to let everyone know we're picked up, even if we think we're in special states.
    // Just in case someone else has missed the memo.  But don't override the special sound that might be already playing!
    if (!wifi.sendData(wifi_event_data_t::PICKED_UP)) {
      Serial.println("Sending picked up failed.");
    }
    prevSendPickedUp = currentMs;
  
    // If we've already been told to play special, don't interrupt with "Picked up" response.
    // SoundPlayer will handle whether or not we're already playing audio.
    if (!player.play(sound_file_t::NORMAL_FILE)) {
      Serial.println("Error: Failed to play norma file.");
    }
  }

  // Check for, and report, the next wifi error.
  struct WifiFailure newErr = wifi.getError();
  if (newErr.type != NO_ERROR) {
    switch(newErr.type) {
      case wifi_error_method_t::RCV:      
        Serial.print("Recieve failed error. Mac: "); Serial.print(newErr.mac[6], HEX);
        break;
      case wifi_error_method_t::RCV_BAD_SIZE:
        Serial.print("Recieve failed due to incorrect message size. Mac: "); Serial.print(newErr.mac[6], HEX);
        break;
      case wifi_error_method_t::SEND_FAILED:
        Serial.print("Send failed. Mac: "); Serial.println(newErr.mac[6], HEX);
        break;
      case wifi_error_method_t::SEND_UNSENT:
        Serial.print("Send not made. Mac: "); Serial.println(newErr.mac[6], HEX);
        break;
      default:
        Serial.print("Unknown wifi error. Mac: "); Serial.println(newErr.mac[6], HEX);
        break;
    }
  }
  
  // Only spam the serial monitor a couple times a second.
  if (currentMs - lastDebugMs < 500) {
    return;
  }
  lastDebugMs = currentMs;
  
  //accel.printDebug();
}