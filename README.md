# clock
## What is clock? 
Clock that only turns on when an object (hand) is close to the device

### Purpose
Clock template for other projects

### Contributors

### Software Dependencies
  - Adafruit_LiquidCrystal https://github.com/adafruit/Adafruit_LiquidCrystal
  - TimeLib https://github.com/PaulStoffregen/Time/blob/master/TimeLib.h
  - DS1307RTC https://github.com/PaulStoffregen/DS1307RTC
  - button https://github.com/ericklein/button
  - VFDTube https://github.com/aguegu/nixie-tube

### BOM
    - Project requires 5v input
    - 6 VFD require dedicated 5v, >=1.3A
    - Works with all Arduino boards, uses I2C pins which are board dependent


### Pinouts


### Information Sources
  - vfdtubes
    http://www.nixieclock.org/?p=411
    http://aguegu.net/?p=1334
    http://www.nixieclock.org/?p=464
    https://www.dfrobot.com/product-831.html

  - Overview of 16x2 LCD panels available at: http://oomlout.com/parts/LCDD-01-guide.pdf
  - Overview of Adafruit i2c/SPI LCD Backpack at: https://learn.adafruit.com/i2c-spi-lcd-backpack/
  - Overview of ChronoDot (RTC) at: http://docs.macetech.com/doku.php/chronodot_v2.0
  - Overview of HC-SR04 (ultrasonic sensor) at: https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE/edit

#### VFD Arduino module

              /----------------------------\
              |                            |
              |                            |
              |                            |
              \----------------------------/
               |                          |
 ------------------------------------------------
 | 5V | 5V | GND | GND | DIN | OE | STCP | SHCP |
 ------------------------------------------------

Two pairs of 5v/GND, one could be used for power in from external power source, the other one could be used as power for Arduino

### Issues
070720 - Code needs to be scrubbed for instances where LCD (row) needs to be cleared before new output
070720 - fix issue where RTC and system time are not being set if RTC has no time stored

### Questions
050417 - are there LCDs with larger character height to facilitate larger text
042617 - how to supress power and built-in LEDs on development boards

### Learnings

### Feature Requests
050417 - ability to change LCD backlight level
070620 - optimize adjustTime to remove redundant code
 
### Revisions
041417 
  - version cloned from current badge to control 6 pin LCD screen
  - version cloned from current button though functionality is not used
  - reading from ChronoDot via simple i2c read
  - ChronoDot time set via example code not in this code base (yet?)
041617
  - button toggles screen text on and off, did not do what I wanted functionality wise
041817
  - Added ultrasonic code which will ultimately trigger clock visibility
042317
  - 041617 - put LCD backlight under programatic control
  - 041817 - integrate ultrasonic into display, dependent on backlight control
  - 041817 - move LCD to I2C
  - button code removed
071517
  - 041417 - read RTC via time library (look at DS3232RTC.h and time.h)
  - 070617 - move code in main to functions
  - 042617 - single digit hours, minutes, seconds to 2 fixed digits
  - 041817 - three digit seconds bug (due to cursor position and time length?) fix
  - 041917 - untrasonic as range finding function returning cm as int
112617
  - 082017 - move comments to readme.md
  - button support to library
062118
  - 062018 - conditional switch between LCD and VFDTube displays
  - improved clock string handling = ClockDigits(x) removed
071618
  - 071617 - ability to modify time
  - 062018 - ability to set time manually
  - 071618 - changes for ButtonHandler library updates
070720
  - general cleanup work for readability
  - [FR]042317 - move serial debug code to conditional compile
  - debug messages to LCD if available