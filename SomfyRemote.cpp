//Implementation of the Somfy Remote System.

#include "Arduino.h"
#include "SomfyRemote.h"


#define SYMBOL 640

#define FRAME_KIND_FIRST 2 //look below for an explanation to 2 and 7.
#define FRAME_KIND_REPEATING 7


SomfyRemote::SomfyRemote(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, 0);
  _pin = pin;
};

uint16_t SomfyRemote::buildFrame(int remote, byte button, uint16_t rollingCode, byte *frame) {
  frame[0] = 0xA7;             // Encryption key. Doesn't matter much
  frame[1] = button << 4;      // Which button did you press? The 4 LSB will be the checksum
  frame[2] = rollingCode >> 8; // Rolling code (big endian)
  frame[3] = rollingCode;      // Rolling code
  frame[4] = remote >> 16;     // Remote address
  frame[5] = remote >>  8;     // Remote address
  frame[6] = remote;           // Remote address

  Serial.print("Frame         : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {  // Displays leading zero in case the most significant nibble is a 0.
      Serial.print("0");
    }
    Serial.print(frame[i], HEX); Serial.print(" ");
  }

  // Checksum calculation: a XOR of all the nibbles
  byte checksum = 0;
  for (byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only

  // Checksum integration
  frame[1] |= checksum; // If a XOR of all the nibbles is equal to 0, the blinds will consider the checksum ok.

  Serial.println(); Serial.print("With checksum : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i], HEX); Serial.print(" ");
  }

  // Obfuscation: a XOR of all the bytes
  for (byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }

  Serial.println(); Serial.print("Obfuscated    : ");
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i], HEX); Serial.print(" ");
  }
  Serial.println(); Serial.print("Rolling Code  : "); Serial.println(rollingCode);
  // We store the value of the rolling code in the EEPROM.
  return (rollingCode + 1);
};

uint16_t SomfyRemote::sendButton(int remote, byte button, uint16_t rollingCode) {
  byte frame[7]; // The fram is 7 long. No need to abstract it.
  uint16_t nextRollingCode = buildFrame(remote, button, rollingCode, frame);

  Serial.println();
  sendCommand(frame, FRAME_KIND_FIRST);
  sendCommand(frame, FRAME_KIND_REPEATING);
  sendCommand(frame, FRAME_KIND_REPEATING);

  return nextRollingCode;
};

//sendLevel/2 will send set _pin to the level during duration
void SomfyRemote::sendLevel(byte level, int duration) {
  digitalWrite(_pin, level);
  delayMicroseconds(duration);
};

//sendLevelChange
void SomfyRemote::sendLevelChange(byte initLevel, int duration1, int duration2) {
  sendLevel( initLevel, duration1);
  sendLevel(!initLevel, duration2);
};

void SomfyRemote::sendCommand(byte *frame, byte sync) {
  if (sync == FRAME_KIND_FIRST) { // Only with the first frame send wake-up pulse & silence
    sendLevelChange(HIGH, 9415, 89565);
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    sendLevelChange(HIGH, 4 * SYMBOL, 4 * SYMBOL);
  }

  // Software sync
  sendLevelChange(HIGH, 4550, SYMBOL);

  // Data: bits are sent one by one, starting with the MSB.
  for (byte i = 0; i < 56; i++) {
    sendLevelChange(((((frame[i / 8] >> (7 - (i % 8))) & 1) == 1) ? LOW : HIGH), SYMBOL, SYMBOL);
  }

  // Inter-frame silence
  sendLevel(LOW, 30415);
};

