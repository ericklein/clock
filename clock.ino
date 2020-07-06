/*
  Project Name:	clock
  Developer:	Eric Klein Jr. (temp2@ericklein.com)
  Description:	template for clock projects

  See README.md for target information, revision history, feature requests, etc.
*/

// Conditional (hardware functionality)
// #define displayVFD
#define displayLCD
//#define buttonEnabled
//#define DEBUG 			// debug messages to serial port

#ifdef displayLCD
	#define EXT_DEBUG	// debug messages to external LCD display
#endif

// time library initialization
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h> // Works with DS3231 as well

//Hardware setup
//buttons
#ifdef buttonEnabled
	#include "buttonhandler.h"
	#define pushButtonHourPin		12
	#define pushButtonMinutePin		11
	#define longButtonPressDelay	3000
	enum { BTN_NOPRESS = 0, BTN_SHORTPRESS, BTN_LONGPRESS };
	ButtonHandler buttonHour(pushButtonHourPin, longButtonPressDelay);
	ButtonHandler buttonMinute(pushButtonMinutePin, longButtonPressDelay);
#endif

// ultrasound
#define triggerPin 7
#define echoPin 6
const byte triggerDistance = 20;  // Distance in cm to toggle LCD backlight

#ifdef displayVFD
	#include "VFDTube.h"
	#define DINPin  8
	#define OEPin   9 //PWM output
	#define STCPPin 10
	#define SHCPPin 11
	#define tubeCount 6
	VFDTube tube(DINPin, OEPin, STCPPin, SHCPPin, tubeCount);
	tube.setBrightness(0x4b); // set brightness, range 0x00 - 0xff [0-255]
#else 
	// LCD display
	#include <Adafruit_LiquidCrystal.h>
	#define lcdRows   2
	#define lcdColumns 16
	Adafruit_LiquidCrystal lcd(0); //i2c connection, default address #0 (A0-A2 not jumpered)
#endif

void setup()
{
	#ifdef DEBUG
		Serial.begin(115200);
		while (!Serial) 
	  	{
	    	delay(1);
	  	}
	#endif

	//Setup ultrasonic sensor
	pinMode(triggerPin, OUTPUT);
	pinMode(echoPin, INPUT);

  	//Setup push buttons
	#ifdef buttonEnabled
		buttonHour.init();
		buttonMinute.init();
  	#endif

	// make sure the RTC has been set previously
	setSyncProvider(RTC.get);
  	if (timeStatus() != timeSet)
  	// Issues 070720: check that elapsed t isn't near zero, aka RTC is awake but isn't set
  	{
		setTime(23,14,00,07,7,2020); //values in the order hr,min,sec,day,month,year
		RTC.set(now());
		#ifdef DEBUG
    		Serial.println("Unable to sync with RTC, time was set via code default");
    	#endif
    	#ifdef EXT_DEBUG
      		displayLCDMessage(0, "Time set in code");
    	#endif
	}
  	else
    {
		#ifdef DEBUG
    		Serial.println("Time provided via RTC");
    	#endif
    	#ifdef EXT_DEBUG
      		displayLCDMessage(0, "RTC set time");
      		delay(2000);
    	#endif
	}

// Prepare LCD
#ifdef displayLCD
	lcd.begin(lcdColumns, lcdRows);
	displayLCDMessage(0,"Current time is:");
#endif
}

void loop()
{
  #ifdef buttonEnabled
  	adjustTime();
  #endif
  //if (timeStatus() == timeSet)
  if (timeSet)
  {
    displayClock();
    toggleBacklight(readDistance());
  } 
  else
  {
  	#ifdef DEBUG
		Serial.println("RTC is either waiting to sync or a serious hardware issue has emerged");
	#endif
	#ifdef EXT_DEBUG
  		displayLCDMessage(1, "RTC wait to sync");
	#endif
  }
}

int readDistance ()
{
    // Returns distance from sensor in centimeters
	long duration;
	int  cm;

	// ultrasonic sensor read
	// clears the triggerPin
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);
	// Sets the triggerPin on HIGH state for 10 microseconds
	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);
	// Reads the echoPin, returns the sound wave travel time in microseconds
	duration = pulseIn(echoPin, HIGH);
	// Distance = (Speed of sound * Time delay) / 2
	// the speed of sound is 343.4 m/s or 0.0343 cm/microsecond to the Temperature of 20°C.
	// inches = (duration/2) / 741;
	cm = duration / 58;
	#ifdef DEBUG
		Serial.print("Distance to object is ");
		Serial.print(cm);
		Serial.println(" cm");
	#endif
	return (cm);
}

void toggleBacklight (int distance)
{
  // Clock display is made visible if object is close enough to clock
  if (distance < triggerDistance)
  {
	#ifdef displayVFD
		tube.setBrightness(0xd0); // set brightness, range 0x00 - 0xff [0-255]
	#else
		lcd.setBacklight(HIGH);
	#endif
	#ifdef DEBUG
	Serial.println("Backlight activated");
	#endif
  }
  else
  {
	#ifdef displayVFD
		tube.setBrightness(0x4b); // set brightness, range 0x00 - 0xff [0-255]
	#else
		lcd.setBacklight(LOW);
	#endif
	#ifdef DEBUG
	Serial.println("Backlight de-activated");
	#endif
  }
}

void displayClock()
{
	char timebuffer[9];
	sprintf(timebuffer, "%02d:%02d:%02d", hour(), minute(), second());

	#ifdef displayVFD
		tube.clear();
		tube.printf("%02d%02d%02d", hour(), minute(), second());
		tube.display();
	#else
		lcd.setCursor(0, 1); // column 0, line 1. line 1 is the second row, since counting begins with 0
		lcd.print(timebuffer);
	#endif
}

#ifdef buttonEnabled
void adjustTime()
// Manually update time via user input
{
	int newHour = hour();
	int newMinute = minute();
	switch (buttonHour.handle())
	{
		case BTN_SHORTPRESS:
		//Debug text
			#ifdef DEBUG
				Serial.println("hour button short press");
			#endif
			if (newHour == 23) 
				newHour = 0;
			else newHour++;
			setTime(newHour,minute(), second(), day(), month(), year());
			if (RTC.chipPresent()) 
				RTC.set(now());
			else
				#ifdef DEBUG
					Serial.println("unable to set RTC time");
				#endif
		break;
		case BTN_LONGPRESS:
			#ifdef DEBUG
				Serial.println("hour button long press");
			#endif
		break;
	}
	switch (buttonMinute.handle())
	{
		case BTN_SHORTPRESS:
			#ifdef DEBUG
				Serial.println("minute button short press");
			#endif
			if (newMinute == 59)
				newMinute = 0;
			else newMinute++;
	    	setTime(hour(),newMinute, second(), day(), month(), year());
			if (RTC.chipPresent())
				RTC.set(now());
			else
	  			#ifdef DEBUG
					Serial.println("unable to set RTC time");
				#endif  
		break;
		case BTN_LONGPRESS:
			#ifdef DEBUG
				Serial.println("minute button long press");
			#endif
		break;
	}
}
#endif

#ifdef EXT_DEBUG
  void displayLCDMessage(int row, String message)
  // first row is zero
  {
    lcd.setCursor(0, row);
    lcd.print(message);
  }
#endif