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
