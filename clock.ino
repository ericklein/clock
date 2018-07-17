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
// #define displayVFD
#define displayLCD

//Pin connections
//buttons
#define pushButtonHourPin  12
#define pushButtonMinutePin  11

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
enum { BTN_NOPRESS = 0, BTN_SHORTPRESS, BTN_LONGPRESS };
#define buttonDelay 10
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
ButtonHandler buttonHour(pushButtonHourPin, buttonDelay);
ButtonHandler buttonMinute(pushButtonMinutePin, buttonDelay);

void setup() {
  Serial.begin(115200);

  //Setup ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Setup push buttons
  buttonHour.init();
  buttonMinute.init();

  while (!Serial) ; // wait until Arduino Serial Monitor opens

      // make sure the RTC has been set previously
      setSyncProvider(RTC.get);
      if (timeStatus() != timeSet)
      {
        Serial.println("Unable to sync with RTC, time needs to be manually updated");
        setTime(18,51,00,22,6,2018); //values in the order hr,min,sec,day,month,year
      }
      else
        Serial.println("RTC has set the time");

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
  adjustTime();
  if (timeStatus() == timeSet)
  {
    displayClock();
    toggleBacklight(readDistance());
  } 
  else Serial.println("RTC is either waiting to sync or a serious hardware issue has emerged");
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
  //Serial.print(cm);
  //Serial.print(" cm; ");
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
    //Serial.println("activating backlight");

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
    //Serial.println("deactiving backlight");
  }
}

void displayClock()
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
  //Serial.println(timebuffer);
}

void adjustTime()
// Manually update time via user input
{
  int newHour = hour();
  int newMinute = minute();
  switch (buttonHour.handle())
  {
  case BTN_SHORTPRESS:
    //Debug text
    Serial.println("hour button short press");
    if (newHour == 23) newHour=0;
    else newHour++;
    setTime(newHour,minute(), second(), day(), month(), year());
  if (RTC.chipPresent()) RTC.set(now());
  else
      Serial.println("Unable to set RTC time");
  break;
  case BTN_LONGPRESS:
    Serial.println("hour button long press"); //debug text
    break;
  }

  switch (buttonMinute.handle())
  {
  case BTN_SHORTPRESS:
  Serial.println("minute button short press"); //debug text
    if (newMinute == 59) newMinute=0;
    else newMinute++;
        setTime(hour(),newMinute, second(), day(), month(), year());
    if (RTC.chipPresent()) RTC.set(now());
    else
      Serial.println("Unable to set RTC time");  
  break;
  case BTN_LONGPRESS:
    Serial.println("minute button long press"); //debug text
    break;
  }
}