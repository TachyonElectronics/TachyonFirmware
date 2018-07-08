/*
	OFFICIAL TACHYON FIRMWARE
	Version 1.x
	
	(c) Martin Hrehor 2018

*/


#ifndef CONF_H_
#define CONF_H_

//Basic settings
#define MIN_BRIGHTNESS 1 //Minimum brigtness level (0-255)
#define BRIGHTNESS_STAY_TIME 1000 //How long does the brigtness bar stay visible for (in ms)
#define CENTER_HOLD_TIME 2500 //How long you have to hold the center button to open the menu (in ms)
#define INCREMENT_TIME 10 // When a button is held, this will specify how fast the edited value decreases/increases
#define BATT_MEASURMENT_INTERVAL 1000 //Interval between battery voltage sampling (in ms)
#define BUTTON_DEBOUNCE_INTERVAL 4 


//Empty magazine flash setup

//Uncomment lines to enable, comment lines to disable

//Comment this line to completely disable this function
#define EMPTY_MAGAZINE_FLASH_INTERVAL 200 

//Flashes the numeric counter
#define EMF_STYLE_NUMBER_FLASH

// >> can be combined with >>

//Flashes the graphical counter
//Use only ONE of the following
//#define EMF_STYLE_BOTTOM_BAR
//#define EMF_STYLE_HAZARDLIGHTS

//Uncomment to make the flashing of the numeric and graphical counter in sync, otherwise, the flashes will be inverted
//#define EMF_SYNC_NUM_AND_BARS



#define _PRESET_COUNT 6


//Button definitions
#define _BTN_NULL 0
#define BTN_LEFT 1
#define BTN_RIGHT 3
#define BTN_CENTER 2
#define BTN_RLD 255

#define BTN_UP BTN_RIGHT //These are just aliases
#define BTN_DOWN BTN_LEFT

#define _BTN_COUNT 5

//Pin definitions (better don't change this)
#define DSP_BL 5
#define DSP_CS 8
#define DSP_RST 10
#define SD_CS 6
#define RLD 1 //PD1 

//Screens
#define SCREEN_MAIN 0
#define SCREEN_SETTINGS 1
#define SCREEN_PRESETS 2
#define SCREEN_COLOR 3
#define SCREEN_RELOADMODES 4
#define SCREEN_UISETUP 5
#define SCREEN_TIME 6
#define SCREEN_SYSINFO 255

//Reload modes
#define RLM_AUTO_OR_MANUAL 0 //Either removing the mag (auto) OR pressing the reload button (manual) will reload ammo count
#define RLM_AUTO_AND_MANUAL 1 //BOTH Manual AND Auto triggers must be activated within a certain time period in order to reload
#define RLM_AUTO_ONLY 2 //self explanatory
#define RLM_MANUAL_ONLY 3 //self explanatory
#define RLM_INVERTED 4 //Removing the mag will reload the gun, unless the reload button is pressed first. If it is pressed, the system will ignore the next mag removal (auto trigger)

//EEPROM
#define _EEPROM_VERIFY_TAG 0x42 //This is a value that is written into address '0' into EEPROM. Upon device bootup, if the value does not match the tag, the device will overwrite the EEPROM to factory settings (useful to prevent bricking should the EEPROM addressing change after a firmware update)
#define EA_BRIGHTNESS 0x01 //EEPROM address for brightness
#define EA_SETTINGS 0x02 //EEPROM address for combined settings struct

//Factory user settings
#define FACT_PRESET1 30
#define FACT_PRESET2 350
#define FACT_UICOLOR 0xFFE0 //Yellow
#define FACT_CTRCOLOR1 0xFFE0 //Yellow
#define FACT_CTRCOLOR2 0xFFE0 //Yellow
#define FACT_CTRCOLOR3 0xFFE0 //Yellow
#define FACT_BRIGHTNESS 100
#define FACT_RLM 0
#define FACT_UIROTATION 0
	
#define FAST_SIGNBIT8(x) (x & 0x80)
#define FAST_SIGNBIT16(x) (x & 0x8000)
#endif /* CONF_H_ */