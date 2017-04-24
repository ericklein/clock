/*
  Project Name : clock
  Developer : Eric Klein Jr. (temp2@ericklein.com)
  Description : Clock that only turns on when an object (hand) is close to the device
  Last Revision Date : 04/23/17
    
  Target
    - Works with all Arduino boards, uses I2C ports which are board dependent
    - Overview of 16x2 LCD panels available at: http://oomlout.com/parts/LCDD-01-guide.pdf
    - Overview of ChronoDot (RTC) at: http://docs.macetech.com/doku.php/chronodot_v2.0
    - Overview of HC-SR04 (ultrasonic sensor) at: https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE/edit
    
  Revisions
  04/14/17 
    - version cloned from current badge to control 6 pin LCD screen
    - version cloned from current button though functionality is not used
    - reading from ChronoDot via simple i2c read
    - ChronoDot time set via example code not in this code base (yet?)
  04/16/17
    - button toggles screen text on and off, did not do what I wanted functionality wise
  04/18/17
    - Added ultrasonic code which will ultimately trigger clock visibility
  04/23/17
    - 041617 - put LCD backlight under programatic control
    - 041817 - integrate ultrasonic into display, dependent on backlight control
    - 041817 - move LCD to I2C
    - button code removed
    

  Feature Requests
    - 041417 - read RTC via time library
    - 041817 - display bug due to cursor position and time length needs to get fix (minutes and seconds)
    - 041917 - untrasonic as range finding function returning cm as int
    - 042317 - implement debug flag for conditional debug messages
    
*/

// Library initialization
#include "Adafruit_LiquidCrystal.h"
#include <Wire.h>

// Assign Arduino pins
#define pushButtonOnePin  7
#define trigPin 12
#define echoPin 13

// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

// Assign global variables
const byte longPressLength = 25;    // Min number of loops for a long press
const byte loopDelay = 20;          // Delay per main loop in ms

enum { EV_NONE = 0, EV_SHORTPRESS, EV_LONGPRESS };

// Class definition

class ButtonHandler {
  public:
    // Constructor
    ButtonHandler(int pin, int longpress_len = longPressLength);

    // Initialization done after construction, to permit static instances
    void init();

    // Handler, to be called in the loop()
    int handle();

  protected:
    boolean was_pressed;     // previous state
    int pressed_counter;     // press running duration
    const int pin;           // pin to which button is connected
    const int longpress_len; // longpress duration
};

ButtonHandler::ButtonHandler(int p, int lp)
  : pin(p), longpress_len(lp)
{
}

void ButtonHandler::init()
{
  pinMode(pin, INPUT_PULLUP);
  was_pressed = false;
  pressed_counter = 0;
}

int ButtonHandler::handle()
{
  int event;
  int now_pressed = !digitalRead(pin);

  if (!now_pressed && was_pressed) {
    // handle release event
    if (pressed_counter < longpress_len)
      event = EV_SHORTPRESS;
    else
      event = EV_LONGPRESS;
  }
  else
    event = EV_NONE;

  // update press running duration
  if (now_pressed)
    ++pressed_counter;
  else
    pressed_counter = 0;

  // remember state, and we're done
  was_pressed = now_pressed;
  return event;
}

// Instantiate button objects
ButtonHandler buttonOne(pushButtonOnePin);

void buttonEvent(const char* button_name, int event)
{
  //short press on buttonOne
  if ((button_name == "buttonOne") && (event == 1))
  {
    Serial.println("short press on button one");
  }
  //long press on buttonOne
  if ((button_name == "buttonOne") && (event == 2))
  {
    Serial.println("long press on button one");
  }
  delay(loopDelay); //debounce
}

void setup() {
  Wire.begin();
  Serial.begin(57600);

  //Setup ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Setup push buttons
  // buttonOne.init();
 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // display initial screen information
  lcd.print("Current time is:");

  // clear /EOSC bit
  // Sometimes necessary to ensure that the clock
  // keeps running on just battery power. Once set,
  // it shouldn't need to be reset but it's a good
  // idea to make sure.
  Wire.beginTransmission(0x68); // address DS3231
  Wire.write(0x0E); // select register
  Wire.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
}

void loop() {

  // read button for events
  //int event1 = buttonOne.handle();

  // deal with button events
  //buttonEvent("buttonOne", event1);

    // send request to receive data starting at register 0
  Wire.beginTransmission(0x68); // 0x68 is DS3231 device address
  Wire.write((byte)0); // start at register 0
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3); // request three bytes (seconds, minutes, hours)
 
  while(Wire.available())
  { 
    int seconds = Wire.read(); // get seconds
    int minutes = Wire.read(); // get minutes
    int hours = Wire.read();   // get hours
 
    seconds = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
    minutes = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
    hours = (((hours & 0b00100000)>>5)*20 + ((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
 
    
  // set the cursor to column 0, line 1. line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  lcd.print(hours); lcd.print(":"); lcd.print(minutes); lcd.print(":"); lcd.print(seconds);
  Serial.print(hours); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);
  }

  // ultrasonic read (test)
   long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  if (distance < 20)
  {  
    Serial.println("turn on the clock");
    lcd.setBacklight(HIGH);
  }
  else
  {
    Serial.println("turn off the backlight");
    lcd.setBacklight(LOW);
  }
    Serial.print(distance);
    Serial.println(" cm");
  delay(1000);
}
