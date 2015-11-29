# Somfy_Remote
An Arduino Sketch able to simulate a Somfy remote control.


[Pushtack decoded the Somfy RTS protocol](https://pushstack.wordpress.com/somfy-rts-protocol/). I used his work to make an Arduino sketch that will emulate a remote.


**How the hardware works:**
Connect a *433.42 Mhz* RF transmitter to Arduino Pin 5 (or change the pin in the sketch). I couldn't find a 433.*42* MHz transmitter so I hacked a current remote. I also ordered 433.42 MHz crystals to change the regular 433.92 MHz I have on my transmitter. Other option would be to use a tunable transmitter (but that hardly looks like the easy way and I'm not a ham radio, so...).


**How the software works:**
What you really want to keep here are the BuildFrame() and SendCommand() procedures. Input the *remote address* and the *rolling code* value and you have a remote. With the sketch, you can send the command through serial line but that would be easily modified to button press or whatever (I plan on running it on an internet-connected ESP8266 to shut the blinds at sunset every day).


**What will be added:**
EEPROM write of the current rolling code. Currently, you loose count as soon as you power off the Arduino. I'm just starting to learn about EEPROM
