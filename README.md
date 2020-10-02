![Image of the licence](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

# Somfy Remote
An Arduino Sketch able to emulate a Somfy remote control.


If you want to learn more about the Somfy RTS protocol, check out [Pushtack](https://pushstack.wordpress.com/somfy-rts-protocol/).



**How the hardware works:**
Connect a *433.42 Mhz* RF transmitter to Arduino Pin 5 (or change the pin in the sketch). I couldn't find a 433.*42* MHz transmitter so I hacked a remote to send my signals. I then ordered 433.42 MHz crystals to change the regular 433.92 MHz ones I have on my transmitters: that's the cheapest way to do it. Other option would be to use a tunable transmitter (but that hardly looks like the easy way and I'm not a ham radio, so...).


**How the software works:**
What you really want to keep here are the BuildFrame() and SendCommand() procedures. Input the *remote address* and the *rolling code* value and you have a remote. With the sketch, you can send the command through serial line but that would be easily modified to button press or whatever (I plan on running it on an internet-connected ESP8266 to shut the blinds at sunset every day).


The rolling code value is stored in the EEPROM, so that you don't loose count of your rolling code after a reset.

<br/>
<br/>
<br/>
**If you want more functionality**, check out the [Python version](https://github.com/Nickduino/Pi-Somfy)
