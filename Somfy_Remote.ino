#define PORT_TX 6 //5 of PORTD = DigitalPin 5

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8

#define REMOTE 0x696120    //<-- Change it!

int rollingCode = 1;       //<-- Change it!

byte frame[7];
byte checksum;

void BuildFrame(byte *frame, int* code, byte button);
void SendCommand(byte *frame, byte sync);


void setup() {
  Serial.begin(115200);
  DDRD |= 1<<PORT_TX; // Pin 5 an output
  PORTD &= !(1<<PORT_TX); // Pin 5 LOW

/*  BuildFrame(frame, HAUT);
  SendCommand(frame, 2);
*/   
}

void loop() {
  if (Serial.available() > 0) {
    char serie = (char)Serial.read();
    Serial.println("");
    if(serie == 'm'||serie == 'u'||serie == 'h') {
      Serial.println("Monte");
      BuildFrame(frame, &rollingCode, HAUT);
    }
    else if(serie == 's') {
      Serial.println("Stop");
      BuildFrame(frame, &rollingCode, STOP);
    }
    else if(serie == 'b'||serie == 'd') {
      Serial.println("Descend");
      BuildFrame(frame, &rollingCode, BAS);
    }
    else if(serie == 'p') {
      Serial.println("Prog");
      BuildFrame(frame, &rollingCode, PROG);
    }
    delay(500); Serial.println("");
    SendCommand(frame, 2);
    for(int i = 0; i<4; i++) {
      SendCommand(frame, 7);
    }
  }
}


void BuildFrame(byte *frame, int* code, byte button) {
//  Serial.print("Pointeur Code : "); Serial.println(*code);
  frame[0] = 0xA7;
  frame[1] = button << 4;
  frame[2] = ((*code) &0xFF00) >> 8; // Rolling code
  frame[3] = (*code) & 0xFF;      // Rolling code
  frame[4] = REMOTE >> 16;
  frame[5] = REMOTE >> 8;
  frame[6] = REMOTE;


  Serial.println(""); Serial.print("Original ");
  for(byte i = 0; i < 7; i++) {
    Serial.print(frame[i],HEX);
  }

  
// Checksum calculation
  checksum = 0;
  frame[1] &= 0b11110000; // Set the Checksum to 0 before doing the calculation
  for(byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum = checksum & 0b1111; // We keep the last 4 bits only

  Serial.println("");
  Serial.print("Checksum RAW : "); Serial.println(checksum);
//Checksum integration
  frame[1] |= checksum;
  Serial.print("Checksum après : ");Serial.println(frame[1] & 0xF);

// Obfuscation
  for(byte i = 1; i < 7; i++) {
    frame[i] = frame[i] ^ frame[i-1];
  }

  Serial.println(""); Serial.print("Obfuscated ");
  for(byte i = 0; i < 7; i++) {
    Serial.print(frame[i],HEX);
  }
  (*code)++; // The rolling code is incremented
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
    if((frame[i/8] >> (7 - (i%8))) & 1 == 1) {
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
