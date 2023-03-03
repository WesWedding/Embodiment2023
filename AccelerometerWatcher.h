#ifndef Accelerometer_Watcher_h
#define Accelerometer_Watcher_h

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <RunningAverage.h>

#include "config.h"

class AccelerometerWatcher {
public:
  AccelerometerWatcher(const short int sampleRate, const short int numSamples, const unsigned long pickedUpDurationMs);
  bool begin(void);
  void update(const unsigned long);
  bool wasPickedUp(void);
  void clearPickedUp(void);
  void printDebug(void);
private:
  bool beginSuccess;
  short int sampleRate;
  short int numSamples;
  unsigned long pickedUpDurationMs;
  unsigned long lastUpdateMs;
  unsigned long pickedUpStart;
  Adafruit_MMA8451 mma;
  RunningAverage accelAve;
  sensors_event_t currentEvent;
  bool pickedUp;

  float totalAcceleration(void);
  float absDiffFromG(float);
};

#endif // Accelerometer_Watcher_h