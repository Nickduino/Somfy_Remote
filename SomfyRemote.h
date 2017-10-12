//Library to emulate a somfy remote.

#ifndef SomfyRemote_h
#define SomfyRemote_h

#include "Arduino.h"

#define REMOTE_STOP  0x1
#define REMOTE_RAISE 0x2
#define REMOTE_LOWER 0x4
#define REMOTE_PROG  0x8

class SomfyRemote {
  public :
    SomfyRemote(int pin = 5);

    //send a command and return the next rolling code
    uint16_t sendButton(int remote, byte button, uint16_t rollingCode);

  private :
    int _pin;

    uint16_t buildFrame(int remote, byte button, uint16_t rollingCode, byte *frame);

    void sendLevel(byte level, int duration);
    void sendLevelChange(byte initState, int duration1, int duration2);
    void sendCommand(byte *frame, byte sync);
};

#endif
