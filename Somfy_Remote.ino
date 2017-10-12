/*   This sketch allows you to emulate a Somfy RTS or Simu HZ remote.
   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

   The rolling code will be stored in EEPROM, so that you can power the Arduino off.

   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)

  Then:
    - m, u or h will make it to go up
    - s make it stop
    - b, or d will make it to go down
    - you can also send a HEX number directly for any weird command you (0x9 for the sun and wind detector for instance)
*/

#include <EEPROM.h>
#include "SomfyRemote.h"

#ifdef ESP8266
#define PORT_TX 2 //the only easily availible pin at the ESP-01
#else
#define PORT_TX 5 //5 of PORTD = DigitalPin 5
#endif

SomfyRemote remote(PORT_TX);
static const int remoteHW = 0x121300; //<-- Change it! 24 bit value.

//We use uint16_t for the rolling code as an unsigned int can be either 2 or 4 bytes depending on the
//target architecture. If the read value is random, we can start from there! Just need to really make
//sure the value is 2 bytes. The Somfy protocol wraps neatly over the end to zero again.

#define EEPROM_ADDRESS_RC 0

//You may need to change these EEPROM funcitons to work with your target architecture
void initEEPROM() {
#ifdef ESP8266
  EEPROM.begin(4); //esp8266 specific
#endif
}

uint16_t getEEPROM() {
  uint16_t rcode;
  EEPROM.get(EEPROM_ADDRESS_RC, rcode);
  return rcode;
}

void putEEPROM(uint16_t code) {
  EEPROM.put(EEPROM_ADDRESS_RC, code);
#ifdef ESP8266
  EEPROM.commit(); //esp8266 specific
#endif
}

byte parseSerial(byte button) {
  byte parsedButton;

  switch (button) {
    case 's' :
      Serial.println("Stop");
      parsedButton = REMOTE_STOP;
      break;
    case 'm' :
    case 'u' :
    case 'h' :
      Serial.println("Raise");
      parsedButton = REMOTE_RAISE;
      break;
    case 'b' :
    case 'd' :
      Serial.println("Descend");
      parsedButton = REMOTE_LOWER;
      break;
    case 'p' :
      Serial.println("Prog");
      parsedButton = REMOTE_PROG;
      break;
    default :
      Serial.println("Custom code");
      parsedButton = button;
  }

  return parsedButton;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  initEEPROM();

  Serial.println();
  Serial.print("Simulated remote number : "); Serial.println(remoteHW, HEX);
  Serial.print("Next rolling code       : "); Serial.println(getEEPROM());
  Serial.println();
}

void loop() {
  if (Serial.available() > 0) {
    putEEPROM(remote.sendButton(remoteHW, parseSerial((byte)Serial.read()), getEEPROM()));
  }
}
