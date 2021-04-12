# 24 hr Clock

## Arduino

### Materials

* Arduino Uno
* ESP8266 ESP-01 Wi-Fi board
* 5V DC to 3.3V DC converter
* Scroll pHAT HD
* Light dependent resistor (LDR)

### Setup

**Hardware**

1. Attach the Scroll pHAT HD to the Arduino.
1. Wire the ESP8266 board using the DC-DC converter to provide power from the Uno's 5V pin.
1. Wire the LDR to analog pin A0.

**Software**

1. In Arduino IDE, install these libraries:
    * Adafruit IS31FL3731 Library
    * ezTime
    * WifiESP
1. Create an `arduino_secrets.h` file alongside the .ino file. In this file, define your Wi-Fi network name and password as follows:
    ```c
    #define SECRET_SSID "<ssid>"
    #define SECRET_PASS "<password>"
    ```
1. Open the .ino file in Arduino IDE and flash it to the Uno.

## Raspberry Pi

### Materials

* Raspberry Pi
* Scroll pHAT HD
* Light dependent resistor (LDR)
* 1uF capacitor

### Setup

**Hardware**

1. Attach the Scroll pHAT HD to the Pi (see [pinout](https://pinout.xyz/pinout/scroll_phat_hd))
1. Wire the LDR and capacitor to GPIO pin #24 (see [instructions](https://gpiozero.readthedocs.io/en/stable/api_input.html#lightsensor-ldr))

**Software**

1. Install dependencies: `pip3 install gpiozero scrollphathd`
1. Run the script: `python3 clock.py`

### Tips

1. To rotate the clock 180 degrees, run `python3 clock.py flip`
1. To run the script on boot, run `crontab -e` and add `@reboot python3 /home/pi/clock.py`
