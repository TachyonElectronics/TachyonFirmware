/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

*/


#ifndef CONF_H_
#define CONF_H_

#include <stdint.h>
//====================Basic settings====================
#define MIN_BRIGHTNESS 1 //Minimum brigtness level (0-255)
#define BRIGHTNESS_STAY_TIME 1000 //How long does the brigtness bar stay visible for (in ms)
#define CENTER_HOLD_TIME 2500 //How long you have to hold the center button to open the menu (in ms)
#define INCREMENT_INTERVAL 100 // When a button is held, this will specify how fast the edited value decreases/increases
#define BRIGHTNESS_INCREMENT_INTERVAL 10 // When a button is held, this will specify how fast the edited value decreases/increases
#define VALUE_EDIT_HOLD_TIME 1000 //When the button is held for this amount of millis, the preset editor change speed will increase
#define BATT_MEASURMENT_INTERVAL 250 //Interval between battery voltage sampling (in ms)
#define BUTTON_DEBOUNCE_INTERVAL 4
#define _PRESET_COUNT 6 //do not change this
#define FACTORY_RESET_CONFIRMATIONS 6 //How many times must the factory reset button be pressed in order to activate
//======================================================


//=================Brightness Settings==================
const uint8_t BRIGHTNESS_LEVELS[] = {1,10,30,80,140,255};
#define BRIGHTNESS_SETTINGS_COUNT sizeof(BRIGHTNESS_LEVELS) / sizeof(BRIGHTNESS_LEVELS[0])

#define BRIGHTNESS_ICON_X 89
#define BRIGHTNESS_ICON_Y 89
#define BRIGHTNESS_ICON_SIZE 12
//======================================================


//====================Ammo Counter Setup====================

//Style
//Uncomment to choose
//#define AC_STYLE_CENTRAL
#define AC_STYLE_CONTINUOUS

/*
#define ACN_POS_Lx 66
#define ACN_SPC_Lx 52
#define ACN_POS_Ly 41

#define ACN_POS_Mx 89
#define ACN_SPC_Mx 44
#define ACN_POS_My 46
*/

//Parent struct; ignore this
struct __digitPositioning{
	uint8_t x, y, spacingx;
	__digitPositioning(uint8_t _x,uint8_t _y,uint8_t _spacingx) : x(_x), y(_y), spacingx(_spacingx){}
};

//Positioning of digit charsets
#define ACN_STARTY 34
const __digitPositioning digitPos[3] = {{14,ACN_STARTY,52}, {1,ACN_STARTY+5,44}, {1,ACN_STARTY+14,32}};

//==========================================================

//====================Empty magazine flash setup====================

//Uncomment lines to enable, comment lines to disable

//Comment this line to completely disable this function
#define EMPTY_MAGAZINE_FLASH_INTERVAL 150

//Flashes the numeric counter
//Beware, this feature is a bit buggy and on rare occasions can cause the counter not to be displayed correctly after reload
//#define EMF_STYLE_NUMBER_FLASH

// >> can be combined with >>

//Flashes the graphical counter
#define EMF_STYLE_AMMO_BAR

//Uncomment to make the flashing of the numeric and graphical counter in sync, otherwise, the flashes will be inverted
//#define EMF_SYNC_NUM_AND_BARS
//=================================================================


//====================Button definitions====================
#define _BTN_NULL 0
#define BTN_LEFT 1
#define BTN_RIGHT 3
#define BTN_CENTER 2
#define BTN_RLD 255

#define _BTN_COUNT 5
//==========================================================

//====================Screens====================
#define SCREEN_MAIN 0
#define SCREEN_SETTINGS1 1
#define SCREEN_PRESETS 2
#define SCREEN_COLOR 3
#define SCREEN_UISETUP 4
#define SCREEN_TIME 5
#define SCREEN_SETTINGS2 6
#define SCREEN_BTSELECT 7
#define SCREEN_AMMOBARSELECT 8
#define SCREEN_EDITVALUE 9
#define SCREEN_MAG_CAL 10
#define SCREEN_DELAYMETER 11
#define SCREEN_SETTINGS3 12
#define SCREEN_COMPASS_SETTINGS 13
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
#define _EEPROM_VERIFY_TAG 0x02 //This is a value that is written into address '0' into EEPROM. Upon device bootup, if the value does not match the tag, the device will overwrite the EEPROM to factory settings (useful to prevent bricking should the EEPROM addressing change after a firmware update)
#define EA_BRIGHTNESS 0x01 //EEPROM address for brightness
#define EA_SETTINGS 0x02 //EEPROM address for combined settings struct
//==============================================

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

//========================Magnetometer settings========================
#define COMPASS_TXT_X 48
#define COMPASS_TXT_Y 88
//=====================================================================

//======================Hop Up Assist Settings=========================
#define HA_START_X 114
#define HA_START_Y 89
#define HA_SIZE_X 10
#define HA_SIZE_Y 14
#define HA_THRESHOLD 0.992F
//=====================================================================

//====================Factory user settings====================
#define FACT_PRESET1 30
#define FACT_PRESET2 350
#define FACT_PRESET3 2000
#define FACT_PRESET4 0
#define FACT_PRESET5 0
#define FACT_PRESET6 0
#define FACT_BATTERY BT_RAW_VOLTAGE
#define FACT_UICOLOR 0xFFE0 //Yellow
#define FACT_CTRCOLOR1 0x07E0 //Green
#define FACT_CTRCOLOR2 0xFFA0 //Orange
#define FACT_CTRCOLOR3 0xF800 //Red
#define FACT_BRIGHTNESS 3
#define FACT_RLM 0
#define FACT_AMMOBARDIR 0
#define FACT_AMMOBAR 1
#define FACT_EMFLASH true //Empty magazine flash

#define FACT_CENTERBTNRELOAD true;

#define FACT_BATTERYCALIBRATION 0

#define FACT_HA_MODE true
#define FACT_COMPASS_MODE 1

#if defined(ARDUINO_TACHYON_V2)
#define FACT_UIROTATION 2
#else
#define FACT_UIROTATION 0
#endif
//=============================================================

//====================Menu Graphics settings===================
#define LIST_MENU_X 8
#define LIST_MENU_Y 0
#define LIST_MENU_W 112
#define LIST_MENU_H 12
#define LIST_MENU_S 4

#define EDITVALUE_VAL_X 128/2-20
#define EDITVALUE_VAL_Y 62
#define EDITVALUE_MSG_X 30
#define EDITVALUE_MSG_Y 48
#define EDITVALUE_BAR_Y 77
//=============================================================

#endif /* CONF_H_ */