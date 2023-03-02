#include "SoundPlayer.h"

SoundPlayer::SoundPlayer(const char* normalName, const char* specialName) : 
  filePlayer(Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS)),
  playingSpecial(false),
  playStart(0) {
    strncpy(normalFileName, normalName, 12);
    strncpy(specialFileName, specialName, 12);
  }

bool SoundPlayer::begin(void) {
  playingSpecial = false;
  bool started = false;
  if (! filePlayer.begin()) {
    Serial.println(F("SoundPlayer:  Error: Failed to find VS1053."));
    started = false;
  } else {
    started = true;
  }

  Serial.println("Sine test");
  filePlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  Serial.println("Sine test complete.");

  filePlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  filePlayer.setVolume(2,2);

  if (! SD.begin(CARDCS)) {
    Serial.println(F("SoundPlayer: Error; SD failed or card not present.  Skipping file verify."));
    // We can't verify files at this point, so aborting.
    return false;
  } else {
    started = started && true;
  }

  if (! verifyFiles()) {
    Serial.println(F("SoundPlayer: Error; expected files not found."));
    started = started && false;
  } else {
    started = started && true;
  }
 
  return started;
}

void SoundPlayer::update(unsigned long currentMs) {
  if (playingSpecial && filePlayer.stopped()) {    
    playingSpecial = false;
  }


  bool potentialFreeze = false;
  unsigned long playDuration = currentMs - playStart;
  // The playback of the device seems to be pretty vulnerable to locking.  This is an attempt to catch two observed coniditons...
  // A reset of the chip is attempted to get things working again going forward.

  // 1. Playback is started but immediately ends
  if (filePlayer.stopped() && playDuration < 700) {
    Serial.println("Detecting failed playback times.");
    filePlayer.reset();
    //filePlayer.softReset();
    playStart = 0;
  }

  // 2.  Playback never "ends" and remains stuck in a "playing" state.
if ( filePlayer.playingMusic && playDuration > UNREASONABLE_PLAYBACK_DURATION) {
  Serial.println("Freeze detected.  Try a reboot!?");
  filePlayer.stopPlaying();
  filePlayer.reset();
  playStart = currentMs;  
}



}

bool SoundPlayer::play(sound_file_t file) {
  char slash[] = "/";
  if (file == sound_file_t::NORMAL_FILE) {
    if (!filePlayer.stopped()) {
      return true;
    }
    
    playStart = millis();
    return filePlayer.startPlayingFile(strcat(slash, normalFileName));
  }
  if (file == sound_file_t::SPECIAL_FILE) {
    if (playingSpecial) {
      //Serial.println("Already playing.");      
      return true;
    }
    playingSpecial = true;
    filePlayer.stopPlaying();
    Serial.println("Playing special.");
    playStart = millis();
    return filePlayer.startPlayingFile(strcat(slash, specialFileName));
  }
  return false;
}

bool SoundPlayer::verifyFiles(void) {

  File dir = SD.open("/");  
  bool normalFound = false;
  bool specialFound = false;

  Serial.println("----- Checking Files ------");

  while(true) {  
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }

    if (entry.isDirectory()) {
      entry.close();
      continue;
    }

    const char* name = entry.name();
    if (!normalFound) {
      Serial.print("Checking...   "); Serial.println(normalFileName);
      if (strcmp(name, normalFileName) == 0) {
        normalFound = true;
        Serial.print("Found normal audio: "); Serial.print(entry.name()); Serial.print("\t \t");
        Serial.println(entry.size(), DEC);  
      }
    }
    if (!specialFound) {
      if (strcmp(name, specialFileName) == 0) {
        specialFound = true;
        Serial.print("Found Special audio: "); Serial.print(entry.name()); Serial.print("\t \t");
        Serial.println(entry.size(), DEC);
      }      
    }
    entry.close();
    if (normalFound && specialFound) {
      // We found what we want.
      break;
    }
  }

  return normalFound && specialFound;
}