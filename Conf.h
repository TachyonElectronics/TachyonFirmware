/*
	OFFICIAL TACHYON FIRMWARE
	Version 1.x
	
	(c) Martin Hrehor 2018

*/


#ifndef CONF_H_
#define CONF_H_

//====================Basic settings====================
#define MIN_BRIGHTNESS 1 //Minimum brigtness level (0-255)
#define BRIGHTNESS_STAY_TIME 1000 //How long does the brigtness bar stay visible for (in ms)
#define CENTER_HOLD_TIME 2500 //How long you have to hold the center button to open the menu (in ms)
#define INCREMENT_TIME 10 // When a button is held, this will specify how fast the edited value decreases/increases
#define INCREMENT_TIME_AMMO 100 // Same as above, applies to preset setup when ammo count is < 100
#define INCREMENT_TIME_AMMO_100 40 // Same as above, applies to preset setup when ammo count is > 100
#define INCREMENT_TIME_AMMO_500 15 // Same as above, applies to preset setup when ammo count is > 1000
#define PRESET_EDIT_HOLD_TIME 750 //When the button is held for this amount of millis, the preset editor change speed will increase
#define BATT_MEASURMENT_INTERVAL 1000 //Interval between battery voltage sampling (in ms)
#define BUTTON_DEBOUNCE_INTERVAL 4 
#define _PRESET_COUNT 6 //do not change this
#define FACTORY_RESET_CONFIRMATIONS 6 //How many times must the factory reset button be pressed in order to activate
//======================================================

//====================Empty magazine flash setup====================

//Uncomment lines to enable, comment lines to disable

//Comment this line to completely disable this function
#define EMPTY_MAGAZINE_FLASH_INTERVAL 150 

//Flashes the numeric counter
//Beware, this feature is a bit buggy and on rare occasions can cause the counter not to be displayed correctly after reload 
//#define EMF_STYLE_NUMBER_FLASH

// >> can be combined with >>

//Flashes the graphical counter
//Use only ONE of the following
#define EMF_STYLE_BOTTOM_BAR
//#define EMF_STYLE_HAZARDLIGHTS

//Uncomment to make the flashing of the numeric and graphical counter in sync, otherwise, the flashes will be inverted
//#define EMF_SYNC_NUM_AND_BARS
//=================================================================


//====================Button definitions====================
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
//==========================================================

//====================Screens====================
#define SCREEN_MAIN 0
#define SCREEN_SETTINGS1 1
#define SCREEN_PRESETS 2
#define SCREEN_COLOR 3
#define SCREEN_RELOADMODES 4
#define SCREEN_UISETUP 5
#define SCREEN_TIME 6
#define SCREEN_SETTINGS2 7
#define SCREEN_BTSELECT 8
#define SCREEN_SYSINFO 255
//===============================================

//	Not currently used - not that practical and only takes up program memory. This could, however, become a compile-time setting
//====================Reload modes====================
#define RLM_AUTO_OR_MANUAL 0 //Either removing the mag (auto) OR pressing the reload button (manual) will reload ammo count
#define RLM_AUTO_AND_MANUAL 1 //BOTH Manual AND Auto triggers must be activated within a certain time period in order to reload
#define RLM_AUTO_ONLY 2 //self explanatory
#define RLM_MANUAL_ONLY 3 //self explanatory
#define RLM_INVERTED 4 //Removing the mag will reload the gun, unless the reload button is pressed first. If it is pressed, the system will ignore the next mag removal (auto trigger)
//====================================================


//====================EEPROM====================
#define _EEPROM_VERIFY_TAG 0x42 //This is a value that is written into address '0' into EEPROM. Upon device bootup, if the value does not match the tag, the device will overwrite the EEPROM to factory settings (useful to prevent bricking should the EEPROM addressing change after a firmware update)
#define EA_BRIGHTNESS 0x01 //EEPROM address for brightness
#define EA_SETTINGS 0x02 //EEPROM address for combined settings struct
//==============================================

//====================Factory user settings====================
#define FACT_PRESET1 30
#define FACT_PRESET2 350
#define FACT_UICOLOR 0xFFE0 //Yellow
#define FACT_CTRCOLOR1 0x07E0 //Green
#define FACT_CTRCOLOR2 0xFFA0 //Orange
#define FACT_CTRCOLOR3 0xF800 //Red
#define FACT_BRIGHTNESS 100
#define FACT_RLM 0
#define FACT_UIROTATION 0
//=============================================================


// ====================Battery measurement settings====================
//Battery types
#define BT_RAW_VOLTAGE 0 //Displays measured voltage directly
#define BT_NIMH_7CELL 1	 //7-Cell Ni-MH (8.4v)
#define BT_NIMH_8CELL 2  //8-Cell Ni-MH (9.6v)
#define BT_LIPO_7V4 3    //7.4v LiPO (2-Cell)
#define BT_LIPO_11V1 4   //11.1v LiPO (3-Cell)
#define BT_StA_9VOLT 5   //Standalone 9v alkaline battery

//Battery percentage calculation is performed through a logistic curve function 
//Following constants can be tweaked to fit various battery types with different discharge curves

//Nickel Metal Hydride
#define NIMH_H_OFFSET 66.21f //A
#define NIMH_K 0.01854f      //k
#define NIMH_BASE 1.872f	 //b
#define NIMH_V_OFFSET 0.002f //c

#define LIPO_H_OFFSET 170.1f
#define LIPO_K 0.02098f
#define LIPO_BASE 1.287f
#define LIPO_V_OFFSET 0

//9v Alkaline (Curve approximated to linear)
#define StA_9V_MAX 9.1f
#define StA_9V_MIN 6.5f

//=====================================================================

#endif /* CONF_H_ */