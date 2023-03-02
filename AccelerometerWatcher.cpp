#include "AccelerometerWatcher.h"
#include <Adafruit_MMA8451.h>

AccelerometerWatcher::AccelerometerWatcher(const short int sampleRate, const short int numSamples, const unsigned long pickedUpDurationMs)
: beginSuccess(false),
  sampleRate(sampleRate),
  numSamples(numSamples),
  pickedUpDurationMs(pickedUpDurationMs),
  lastUpdateMs(0),
  pickedUpStart(0),
  accelAve(numSamples),
  currentEvent(),
  pickedUp(false) {}

bool AccelerometerWatcher::begin(void) {
  if (! mma.begin()) {
    return false;
  }
  beginSuccess = true;

  mma.setRange(MMA8451_RANGE_2_G);

  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");  

  return true;
}

void AccelerometerWatcher::update(const unsigned long millis) {
  if (!beginSuccess) return;  // Strangely, reads and getEvent will all return values even if the sensor failed to begin().
  
  mma.read();
  const unsigned long timeSinceSampleMs = millis - lastUpdateMs;

  /* Get a new sensor event */ 
  bool res = mma.getEvent(&currentEvent);
  if (!res) return;
  
  if (timeSinceSampleMs > sampleRate) {
    accelAve.add(totalAcceleration());
    lastUpdateMs = millis;
  }

  // Have we been "picked up" long enough to reset?
  // Sanity check: millis() can loop back to zero if device has been running
  // for too long.  Best to just reset timing at that point to avoid
  // getting stuck in "picked up."
  if (pickedUp && pickedUpStart > millis) {
    pickedUp = false;
    pickedUpStart = 0;
  }
  const unsigned long pickedUpEnd = pickedUpStart + pickedUpDurationMs;
  if (pickedUp) {
    if (pickedUpEnd <= millis) {
      pickedUp = false;
    }   
  } else {
    float ave = accelAve.getAverage();
    if (absDiffFromG(ave) > ACCEL_THRESH_FROM_G) {
      pickedUp = true;
    }    
  }
}

float AccelerometerWatcher::totalAcceleration(void) {
  // Should I be doing "Fast Inverse Square" stuff from Quake 3 here?
  return sqrt(sq(currentEvent.acceleration.x) + sq(currentEvent.acceleration.y) + sq(currentEvent.acceleration.z));
}

float AccelerometerWatcher::absDiffFromG(float acceleration) {
  return abs(acceleration - 9.806);
}

bool AccelerometerWatcher::wasPickedUp(void) {
  return pickedUp;
}
void AccelerometerWatcher::clearPickedUp(void) {
  pickedUp = false;
  accelAve.clear();
}

void AccelerometerWatcher::printDebug(void) {

  Serial.print("X:\t"); Serial.print(mma.x); 
  Serial.print("\tY:\t"); Serial.print(mma.y); 
  Serial.print("\tZ:\t"); Serial.print(mma.z); 
  Serial.println();

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: \t"); Serial.print(currentEvent.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(currentEvent.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(currentEvent.acceleration.z); Serial.print("\t");
  Serial.print("Total: \t"); Serial.print(totalAcceleration()); Serial.print("\t");
  Serial.print("Ave: \t"); Serial.print(accelAve.getAverage()); Serial.print("\t");
  Serial.print("Diff from gravity: \t");  Serial.print(absDiffFromG(accelAve.getAverage())); Serial.print("\t");
  Serial.println("m/s^2 ");
  
  /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
  
  switch (o) {
    case MMA8451_PL_PUF: 
      Serial.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB: 
      Serial.println("Portrait Up Back");
      break;    
    case MMA8451_PL_PDF: 
      Serial.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB: 
      Serial.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF: 
      Serial.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB: 
      Serial.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF: 
      Serial.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB: 
      Serial.println("Landscape Left Back");
      break;
    }
  Serial.println();  
}