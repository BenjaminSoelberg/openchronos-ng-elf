/*
    modules/clock.c: clock module for openchronos-ng

    Copyright (C) 2012 Thomas Post <openchronos-ng@post-net.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <openchronos.h>

/* driver */
#include <drivers/rtca.h>
#include <drivers/display.h>

struct Tide {
	uint8_t hoursLeft;	//hours left to next low tide
	uint8_t minutesLeft; //minutes left to next low tide
};

typedef enum {
	TideDisplayState_Graph = 0,
	TideDisplayState_ToLowCounter,
	TideDisplayState_ToHighCounter,
	//editing mode
	TideDisplayState_Editing,
} TideDisplayState;

//time to the next low tide
struct Tide tide;

const uint16_t twentyFourHoursInMinutes = (uint32_t)1440;
const uint16_t fullTideTime = (uint16_t)754;
const uint16_t halfTideTime = (uint16_t)372;//.5;

//state of the display
TideDisplayState displayState = TideDisplayState_Graph;
static struct lcd_screen screen[3];
static struct lcd_screen *currentScreen;

uint8_t* graphs[4] = {
  (uint8_t*)"_[^]_",
  (uint8_t*)"[^]_[",
  (uint8_t*)"^]_[^",
  (uint8_t*)"]_[^]"
};
uint8_t graphOffset = 0;

//editing state
struct Tide enteredTimeOfNextLow;
uint8_t fieldBeeingEdited = 0;


// MARK: Helper Functions
uint16_t timeInMinutes(struct Tide aTide) {
	uint16_t result = (uint16_t)aTide.minutesLeft;
	result += (uint16_t)aTide.hoursLeft * (uint16_t)60;
	return result;
}

uint16_t timeNowInMinutes() {
	struct Tide timeNow;
	uint8_t hour, min, sec;
	//get time from the system
	rtca_get_time(&hour, &min, &sec);
	
	timeNow.hoursLeft = hour;
	timeNow.minutesLeft = min;
	if(sec > 30) min++;
	return timeInMinutes(timeNow);
}

struct Tide timeFromMinutes(uint16_t minutes) {
	struct Tide newTide;
	
	newTide.hoursLeft = minutes/60;
	
	uint16_t hoursInMinutes = (uint16_t)newTide.hoursLeft*(uint32_t)60;
	minutes -= hoursInMinutes;
	newTide.minutesLeft = minutes;
	
	return newTide;
}

// MARK: Drawing

void drawScreen() 
{
  display_clear(NULL, 1);
	display_clear(NULL, 2);
	
  uint16_t nowInMinutes = timeNowInMinutes();
  uint16_t leftUntilLow = timeInMinutes(tide);
  uint16_t leftUntilHigh = halfTideTime;
  if (leftUntilLow > leftUntilHigh) {	
    leftUntilHigh = leftUntilLow - leftUntilHigh;
  } else {
    leftUntilHigh = leftUntilLow + leftUntilHigh;
  }
  struct Tide lowTide = tide;
  struct Tide highTide = timeFromMinutes(leftUntilHigh);
  
  //screen 0 //graph + time till next peak 
  //line1 time
  if(leftUntilHigh < leftUntilLow) {
    //show time till high
    display_chars(&screen[0], LCD_SEG_L1_3_2, _itoa(highTide.hoursLeft, 2, 1), SEG_SET);
    display_chars(&screen[0], LCD_SEG_L1_1_0, _itoa(highTide.minutesLeft, 2, 0), SEG_SET);
    display_symbol(&screen[0], LCD_SYMB_MAX, SEG_SET);
    
  } else {
    //show time till low
    display_chars(&screen[0], LCD_SEG_L1_3_2, _itoa(lowTide.hoursLeft, 2, 1), SEG_SET);
    display_chars(&screen[0], LCD_SEG_L1_1_0, _itoa(lowTide.minutesLeft, 2, 0), SEG_SET);
    display_symbol(&screen[0], LCD_UNIT_L2_MI, SEG_SET);
  }
  display_symbol(&screen[0], LCD_SEG_L1_COL, SEG_SET);
  display_symbol(&screen[0], LCD_SEG_L1_COL, BLINK_ON);
  
  //line 2 graph
  display_chars(&screen[0], LCD_SEG_L2_4_0, (uint8_t*)graphs[graphOffset], SEG_SET);
  display_symbol(&screen[0], LCD_SEG_L2_COL1, SEG_ON);
  
  
  //screen 1
  //line 1 time till low
  display_chars(&screen[1], LCD_SEG_L1_3_2, _itoa(lowTide.hoursLeft, 2, 1), SEG_SET);
  display_chars(&screen[1], LCD_SEG_L1_1_0, _itoa(lowTide.minutesLeft, 2, 0), SEG_SET);
  display_symbol(&screen[1], LCD_UNIT_L2_MI, SEG_SET);
  
  //line 2 calculate time of next high
  uint16_t lowTideTimeInMinutes = (nowInMinutes + leftUntilLow) % twentyFourHoursInMinutes;
  struct Tide lowTideTime = timeFromMinutes(lowTideTimeInMinutes);
  display_chars(&screen[1], LCD_SEG_L2_3_2, _itoa(lowTideTime.hoursLeft, 2, 1), SEG_SET);
  display_chars(&screen[1], LCD_SEG_L2_1_0, _itoa(lowTideTime.minutesLeft, 2, 0), SEG_SET);
    
    
  // screen 2  
  // Line 1 time high
  display_chars(&screen[2], LCD_SEG_L1_3_2, _itoa(highTide.hoursLeft, 2, 1), SEG_SET);
  display_chars(&screen[2], LCD_SEG_L1_1_0, _itoa(highTide.minutesLeft, 2, 0), SEG_SET);
  display_symbol(&screen[2], LCD_SYMB_MAX, SEG_SET);
  
  //line 2 calculate time of next high
  uint16_t highTideTimeInMinutes = (nowInMinutes + leftUntilHigh) % twentyFourHoursInMinutes;
  struct Tide highTideTime = timeFromMinutes(highTideTimeInMinutes);
  display_chars(&screen[2], LCD_SEG_L2_3_2, _itoa(highTideTime.hoursLeft, 2, 1), SEG_SET);
  display_chars(&screen[2], LCD_SEG_L2_1_0, _itoa(highTideTime.minutesLeft, 2, 0), SEG_SET);
  
  lcd_screen_virtual_to_real(currentScreen);
}

// MARK: System Bus Events
void minuteTick()
{
  uint32_t tideInMinutes =  timeInMinutes(tide);
  graphOffset = (fullTideTime - tideInMinutes)/((uint16_t)186);
  graphOffset = graphOffset % 4;
  
  //count down the timer
  if(tide.minutesLeft == 0) {
    tide.minutesLeft = 60;
    if(tide.hoursLeft == 0) {
      //TODO: change to full tide time
      tide = timeFromMinutes(fullTideTime+1);
      return;
    }
    tide.hoursLeft--;
  }
  tide.minutesLeft--;
  
  //draw screens
  drawScreen();
}

// MARK: - Time Setup

void displayEditValue()
{
	display_clear(NULL, 1);
	display_clear(NULL, 2);
	
  switch(fieldBeeingEdited) {
    case 0:
      display_chars(NULL, LCD_SEG_L1_3_2, _itoa(enteredTimeOfNextLow.hoursLeft, 2, 0), SEG_ON);
      display_chars(NULL, LCD_SEG_L1_1_0, _itoa(enteredTimeOfNextLow.minutesLeft, 2, 0), SEG_ON);
      display_chars(NULL, LCD_SEG_L1_3_2, NULL, BLINK_ON);
      display_chars(NULL, LCD_SEG_L1_1_0, NULL, BLINK_OFF);
      break;
    case 1:
      display_chars(NULL, LCD_SEG_L1_3_2, _itoa(enteredTimeOfNextLow.hoursLeft, 2, 0), SEG_ON);
      display_chars(NULL, LCD_SEG_L1_1_0, _itoa(enteredTimeOfNextLow.minutesLeft, 2, 0), SEG_ON);
      display_chars(NULL, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
      display_chars(NULL, LCD_SEG_L1_1_0, NULL, BLINK_ON);
      break;
    default:
      break;
  }
}

void changeValue(int8_t amount)
{
  switch(fieldBeeingEdited) {
    case 0:
      enteredTimeOfNextLow.hoursLeft = (enteredTimeOfNextLow.hoursLeft + amount) % 24;
      break;
    case 1:
      enteredTimeOfNextLow.minutesLeft = (enteredTimeOfNextLow.minutesLeft + amount) % 60; 
      break;
    default:
      break;
  }
  displayEditValue();
}

void startEditingNextValue()
{
  fieldBeeingEdited = (fieldBeeingEdited + 1) % 2;
  displayEditValue();
}

void endEditing()
{
  tide = enteredTimeOfNextLow;
	display_clear(NULL, 1);
	display_clear(NULL, 2);
	drawScreen();
}

// MARK:  - Buttons
void longNumberButton() 
{
  //clear screen
  display_clear(NULL, 1);
	display_clear(NULL, 2);
	
	enteredTimeOfNextLow = tide;
  menu_editmode_start(&changeValue, &startEditingNextValue, &endEditing);
}

void buttonUp()
{
  displayState = (displayState + 1 ) % 3;
  currentScreen = &screen[displayState];
  drawScreen();
}

void buttonDown() 
{
  if(displayState <= 0) {
    displayState = 2;
  } else {
    displayState = (displayState - 1 ) % 3;
  }
  currentScreen = &screen[displayState];
  drawScreen();
}

// MARK: - Activate and Deactivate
void activate()
{
  	/* create three empty screens */
	lcd_screen_create(&screen[0]);
	lcd_screen_create(&screen[1]);
	lcd_screen_create(&screen[2]);
	
	displayState = TideDisplayState_Graph;
	currentScreen = &screen[displayState];
	drawScreen();
}

void deactivate()
{
  /* destroy virtual screens */
	lcd_screen_destroy(&screen[0]);
	lcd_screen_destroy(&screen[1]);
	lcd_screen_destroy(&screen[2]);
		
	display_clear(NULL, 1);
	display_clear(NULL, 2);
}

void tide_init()
{   
  sys_messagebus_register(&minuteTick, SYS_MSG_RTC_MINUTE);
  menu_add_entry(&buttonUp, &buttonDown, NULL, NULL, &longNumberButton, NULL, &activate, &deactivate);
  tide = timeFromMinutes(90);//fullTideTime);
  minuteTick(); //initla display setup
}