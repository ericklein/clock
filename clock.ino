/*
  Project Name:   clock
  Developer:      Eric Klein Jr. (temp2@ericklein.com)
  Description:    Clock that only turns on when an object (hand) is close to the device

  See README.md for target information, revision history, feature requests, etc.
*/

// Library initialization
#include <Adafruit_LiquidCrystal.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h> // Works with DS3231 as well
#include "buttonhandler.h"
#include "VFDTube.h"

// Conditional code, comment out the display not in use
#define displayVFD
#define displayLCD

// Pin connections
// buttons
//#define pushButtonOnePin  7

// ultrasound
#define trigPin 7
#define echoPin 6

#ifdef displayVFD
#define DINPin  8
#define OEPin   9 //PWM output
#define STCPPin 10
#define SHCPPin 11
#endif

// Global variables
const byte triggerDistance = 20;  // Distance in centimeters to turn on and off the LCD backlight
#ifdef displayVFD
#define tubeCount 6
#endif

#ifdef displayLCD
#define lcdRows   2
#define lcdColumns 16
#endif

// Display connections
#ifdef displayLCD
Adafruit_LiquidCrystal lcd(0); //i2c connection, default address #0 (A0-A2 not jumpered)
#endif
#ifdef displayVFD
VFDTube tube(DINPin, OEPin, STCPPin, SHCPPin, tubeCount);
#endif

// Instantiate button objects
//ButtonHandler buttonOne(pushButtonOnePin);

void setup() {
  Serial.begin(115200);

  //Setup ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Setup push buttons
  //buttonOne.init();

  while (!Serial) ; // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  // Prepare LCD
#ifdef displayLCD
  lcd.begin(lcdColumns, lcdRows);
  lcd.print("Current time is:");
#endif

  // Prepare vfd
#ifdef displayVFD
  tube.setBrightness(0x4b); // set brightness, range 0x00 - 0xff [0-255]
#endif
}

void loop()
{
  /*switch (buttonOne.handle()) {
    case BTN_SHORTPRESS:
      Serial.println("button short press"); //debug text
      break;
    case BTN_LONGPRESS:
      Serial.println("button long press"); //debug text
      break;
    } */

  if (timeStatus() == timeSet) {
    ClockDisplay();
    toggleBacklight(readDistance());
  } else {
    Serial.println("Please set RTC clock time for this solution to work properly");
    delay(4000);
  }
  delay(1000);
}

int readDistance ()
{
  long duration;
  int  cm;

  // Returns distance from sensor in centimeters
  // ultrasonic sensor read
  // clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Distance = (Speed of sound * Time delay) / 2
  // the speed of sound is 343.4 m/s or 0.0343 cm/microsecond to the Temperature of 20°C.
  //   inches = (duration/2) / 741;
  cm = duration / 58;
  Serial.print(cm);
  Serial.print(" cm; ");
  return (cm);
}

void toggleBacklight (int distance)
{
  // Clock display is made visible if object is close enough to clock
  if (distance < triggerDistance)
  {
    //lcd
#ifdef displayLCD
    lcd.setBacklight(HIGH);
#endif
    //vfd
#ifdef displayVFD
    tube.setBrightness(0xd0); // set brightness, range 0x00 - 0xff [0-255]
#endif
    Serial.println("activating backlight");

  }
  else
  {
    //lcd
#ifdef displayLCD
    lcd.setBacklight(LOW);
#endif
    //vfd
#ifdef displayVFD
    tube.setBrightness(0x4b); // set brightness, range 0x00 - 0xff [0-255]
#endif
    Serial.println("deactiving backlight");
  }
}

void ClockDisplay()
{
  char timebuffer[9];
  sprintf(timebuffer, "%02d:%02d:%02d", hour(), minute(), second());
  // LCD
#ifdef displayLCD
  lcd.setCursor(0, 1); // column 0, line 1. line 1 is the second row, since counting begins with 0
  lcd.print(timebuffer);
#endif

  // vfd
#ifdef displayVFD
  tube.clear();
  tube.printf("%02d%02d%02d", hour(), minute(), second());
  tube.display();
#endif

  // debug time display
  Serial.println(timebuffer);
}

/*

  Code to manually change the time via buttons, see 041417 checkin and 071617 requests re: change time


  void TimeAdjust(){
  int buttonH = digitalRead(5);
  int buttonM = digitalRead(4);
  if (buttonH == LOW || buttonM == LOW){
    delay(500);
    tmElements_t Now;
    RTC.read(Now);
    int hour=Now.Hour;
    int minutes=Now.Minute;
    int second =Now.Second;
      if (buttonH == LOW){
        if (Now.Hour== 23){Now.Hour=0;}
          else {Now.Hour += 1;};
        }else {
          if (Now.Minute== 59){Now.Minute=0;}
          else {Now.Minute += 1;};
          };
    RTC.write(Now);
    }
  }*/
