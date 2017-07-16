/*
  Project Name : clock
  Developer : Eric Klein Jr. (temp2@ericklein.com)
  Description : Clock that only turns on when an object (hand) is close to the device
  Last Revision Date : 07/15/17
    
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
  07/15/17
    - 041417 - read RTC via time library (look at DS3232RTC.h and time.h)
    - 070617 - move code in main to functions
    - 042617 - single digit hours, minutes, seconds to 2 fixed digits
    - 041817 - three digit seconds bug (due to cursor position and time length?) fix
    - 041917 - untrasonic as range finding function returning cm as int

  Feature Requests
    - 042317 - implement debug flag for conditional debug messages
  	- 042617 - supress power and built-in LEDs
  	- 050417 - Larger text for time (change screens)
  	- 050417 - Need to change LCD backlight level
    - 070617 - comment all Serial debug code when code is stable again
    
*/

// Library initialization
#include <Adafruit_LiquidCrystal.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h> // Works with DS3231 as well

// Pin connections
#define pushButtonOnePin  7
#define trigPin 12
#define echoPin 11
// SDL
// CLK

// LCD connection via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

// Global variables
const byte longPressLength = 25;    // Min number of loops for a long press
const byte loopDelay = 20;          // Delay per main loop in ms
const byte triggerDistance = 20;	// Distance in centimeters to turn on and off the LCD backlight
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
  Serial.begin(57600);

  //Setup ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Setup push buttons
  // buttonOne.init();

  while (!Serial) ; // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");
 
  // set LCD columns and rows
  lcd.begin(16, 2);
  // display initial screen information
  lcd.print("Current time is:");
}

void loop()
{
  // read button for events
  //int event1 = buttonOne.handle();

  // deal with button events
  //buttonEvent("buttonOne", event1);

  if (timeStatus() == timeSet) {
    digitalClockDisplay();
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
  cm = duration/58;
  Serial.print(cm);
  Serial.print(" cm; ");
  return(cm);
}

void toggleBacklight (int distance) 
{
  // Clock display is made visible if object is close enough to clock
  if (distance < triggerDistance)
  {  
    Serial.println("activating backlight");
    lcd.setBacklight(HIGH);
  }
  else
  {
    Serial.println("disabling backlight");
    lcd.setBacklight(LOW);
  }
}

void digitalClockDisplay()
{
  //displays time on LCD clock
  // set the cursor to column 0, line 1. line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // display time
  lcd.print(hour());
  clockDigits(minute());
  clockDigits(second());
  // debug time display
  Serial.print(hour()); Serial.print(":"); Serial.print(minute()); Serial.print(":"); Serial.println(second());
  // Display date
  //Serial.print(day());
  //Serial.print(" ");
  //Serial.print(month());
  //Serial.print(" ");
  //Serial.print(year()); 
  //Serial.println(); 
}

void clockDigits(int digits)
{
  // function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}
