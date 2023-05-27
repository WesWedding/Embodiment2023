# Team No Listen's Embodiment 2023
Supporting software for the team's submission to the NWACC 2023 Spring Arts and Culture Festival.

## Microcontroller Driver
This software is intended for an ESP8266 chipset on an Arduino architecture, and was specifically
implemented for Adafruit's Feather HUZZAH ESP8266 board with an attached Music Maker FeatherWing w/ Amp and Adafruit MMA8451.

This controller lived inside 3 different devices that could be picked up by visitors to the show,
and if all 3 devices were picked up at once they simultaneously reacted with a special sound and vibration.


## Dependencies
# From Arduino Library Manager
* [RingBufCPP](https://github.com/wizard97/Embedded_RingBuf_CPP) - A Ring Buffer implementation handy for interrupt (ISR) programming.  Not to be confused with the builtin Ringbuf, which is about reading log files!

## Troubleshooting

I had to uninstall/delete the official SDFat library (<MYDOCS>/Arduino/libraries/sdfat) in order for this to compile, due to conflicts relating to the ESP8266's own SdFat lib.

### Device acts like it's picked up constantly

If the device is spitting out "picked up" events constantly, one cause could be that the accelerometer is malfunctioning or has been disconnected after startup.

### No Playback

#### Transducers
The wires on the transducers are extremely fragile.  Check  
