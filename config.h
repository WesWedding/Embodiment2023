#ifndef Config_h
#define Config_h

// Define some things for Accelerometer usage
// SENSE_SAMPLE_RATE * SENSE_AVERAGE_COUNT = DURATION OF MEASUREMENT.  E.g 10 samples done every 100ms is 1 second of rolling ave.
#define SENSE_SAMPLE_RATE       100  // How many millisecs to wait between samples
#define SENSE_AVERAGE_COUNT       5  // How big should the running average get; how long should the average cover?
#define SENSE_PICKUP_DURATION 3000  // Milliseconds that an object should count as "held"
#define ACCEL_G               9.806  // What counts as "G."  The accelerometer might report a different G baseline than what is actually G.
#define ACCEL_THRESH_FROM_G   1.0   // How far away from 9.8 can we get?

// Define the files we need to play
#define SOUND_NORMAL_FILE   "NORMAL.MP3"
#define SOUND_SPECIAL_FILE   "SPECIAL.MP3"

// Define the mac addresses for the 3 devices.
#define MAC_ADDR_1  {0xCA, 0xC9, 0xA3, 0x90, 0x37, 0x2B}
#define MAC_ADDR_2  {0xCA, 0xC9, 0xA3, 0x90, 0x36, 0x33}
#define MAC_ADDR_3  {0xCA, 0xC9, 0xA3, 0x8F, 0x89, 0xAA}


#endif // Config_h