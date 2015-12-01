/*  This sketch allows you to simulate a Somfy RTS or Simu HZ remote.
   The rolling code will be stored in EEPROM, so that you can power the Arduino off.
   
   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)
*/

#include <EEPROM.h>
#define PORT_TX 5 //5 of PORTD = DigitalPin 5

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8
#define EEPROM_ADDRESS 0

#define REMOTE 0x698140    //<-- Change it!

unsigned int newRollingCode = 101;       //<-- Change it!
unsigned int rollingCode = 0;
byte frame[7];
byte checksum;

void BuildFrame(byte *frame, byte adresse, byte button);
void SendCommand(byte *frame, byte sync);


void setup() {
  Serial.begin(115200);
  DDRD |= 1<<PORT_TX; // Pin 5 an output
  PORTD &= !(1<<PORT_TX); // Pin 5 LOW

  if (EEPROM.get(EEPROM_ADDRESS, rollingCode) < newRollingCode) {
    EEPROM.put(EEPROM_ADDRESS, newRollingCode);
  }
  Serial.print("Simulated remote number : "); Serial.println(REMOTE, HEX);
  Serial.print("Current rolling code    : "); Serial.println(rollingCode);
}

void loop() {
  if (Serial.available() > 0) {
    char serie = (char)Serial.read();
    Serial.println("");
//    Serial.print("Remote : "); Serial.println(REMOTE, HEX);
    if(serie == 'm'||serie == 'u'||serie == 'h') {
      Serial.println("Monte");
      BuildFrame(frame, HAUT);
    }
    else if(serie == 's') {
      Serial.println("Stop");
      BuildFrame(frame, STOP);
    }
    else if(serie == 'b'||serie == 'd') {
      Serial.println("Descend");
      BuildFrame(frame, BAS);
    }
    else if(serie == 'p') {
      Serial.println("Prog");
      BuildFrame(frame, PROG);
    }
    else {
      Serial.println("Custom");
      BuildFrame(frame, serie);
    }
    delay(500); // Is it useful?
    Serial.println("");
    SendCommand(frame, 2);
    for(int i = 0; i<4; i++) {
      SendCommand(frame, 7);
    }
  }
}


void BuildFrame(byte *frame, byte button) {
  unsigned int code;
  EEPROM.get(EEPROM_ADDRESS, code);
  frame[0] = 0xA7;
  frame[1] = button << 4;
  frame[2] = code >> 8; // Rolling code
  frame[3] = code;      // Rolling code
  frame[4] = REMOTE >> 16;
  frame[5] = REMOTE >> 8;
  frame[6] = REMOTE;

  Serial.print("Frame      : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  
// Checksum calculation
  checksum = 0;
  frame[1] &= 0b11110000; // Set the Checksum to 0 before doing the calculation
  for(byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum = checksum & 0b1111; // We keep the last 4 bits only


//Checksum integration
  frame[1] |= checksum;

  Serial.println(""); Serial.print("Frame+Chk  : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }

  
// Obfuscation
  for(byte i = 1; i < 7; i++) {
    frame[i] = frame[i] ^ frame[i-1];
  }

  Serial.println(""); Serial.print("Obfuscated : ");
  for(byte i = 0; i < 7; i++) {
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Rolling Code : "); Serial.println(code);
  EEPROM.put(EEPROM_ADDRESS, code + 1);
}

void SendCommand(byte *frame, byte sync) {

  if(sync == 2) {
  //Wake-up pulse & Silence
    PORTD |= 1<<PORT_TX;
    delayMicroseconds(9415);
    PORTD &= !(1<<PORT_TX);
    delayMicroseconds(89565);
  } // Only with the first frame
  

// Hardware sync
  for (int i = 0; i < sync; i++) {
//    Serial.print("Hardware sync ");Serial.println(i);
    PORTD |= 1<<PORT_TX;
    delayMicroseconds(4*SYMBOL);
    PORTD &= !(1<<PORT_TX);
    delayMicroseconds(4*SYMBOL);
  } // Two sync for the first frame, seven for the following.

// Software sync
  PORTD |= 1<<PORT_TX;
  delayMicroseconds(4550);
  PORTD &= !(1<<PORT_TX);
  delayMicroseconds(SYMBOL);
  
  
//Data
for(byte i = 0; i < 57; i++) {
    if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
      PORTD &= !(1<<PORT_TX);
      delayMicroseconds(SYMBOL);
      PORTD ^= 1<<5;
      delayMicroseconds(SYMBOL);
    }
    else {
      PORTD |= (1<<PORT_TX);
      delayMicroseconds(SYMBOL);
      PORTD ^= 1<<5;
      delayMicroseconds(SYMBOL);
    }
  }
  
  PORTD &= !(1<<PORT_TX);
  delayMicroseconds(30415); // Inter-frame silence
}
