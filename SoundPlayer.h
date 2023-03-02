#ifndef Sound_Player_h
#define Sound_Player_h

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// Define Pins for the Wing Board (cribbed from the feather_player demo sketch)
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS      16     // VS1053 chip select pin (output)
#define VS1053_DCS     15     // VS1053 Data/command select pin (output)
#define CARDCS          2     // Card chip select pin
#define VS1053_DREQ     0     // VS1053 Data request, ideally an Interrupt pin

#define UNREASONABLE_PLAYBACK_DURATION 10000

enum sound_file_t {
  NORMAL_FILE = 0,
  SPECIAL_FILE,
};

class SoundPlayer {
public:
  SoundPlayer(const char* normalName, const char* specialName);
  bool begin(void);
  void update(unsigned long currentMs);
  bool play(sound_file_t file);

private:
  Adafruit_VS1053_FilePlayer filePlayer;
  bool playingSpecial;
  unsigned long playStart;

  bool verifyFiles(void);
  char normalFileName[13];
  char specialFileName[13];
};

#endif // Sound_Player_h
