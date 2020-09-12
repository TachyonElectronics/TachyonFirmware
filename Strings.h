/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

STRINGS File (English)
*/


#ifndef STRINGS_H_
#define STRINGS_H_

//PROGMEM strings

/*

|=====!!!===IMPORTANT===!!!=====|
|							    |
| When Modifying the firmware,  |
| be sure to change the version |
| string to prevent confusion   |
| and to protect your copyright!|
| 							    |
|===============================|


*/

//ALWAYS keep the 'VER?' tag at the beginning!!! This is used by external software (such as Tachyon Utils) to determine the version from a compiled binary file
const PROGMEM char _Str_Version[] = "VER?3.0"; //Official Default Firmware

#define VERSION PROGMEMSTRING(_Str_Version + 4) //Macro used to cut off the 'VER?' tag

//MAXIMUM NUMBER OF CHARACTERS FOR MENU ITEM: 18

const PROGMEM char Str_Back[] = "<< Back";

const PROGMEM char Str_ExitSettings[] = "<< Save & Exit";
const PROGMEM char Str_ManagePresets[] = "Manage Presets";
const PROGMEM char Str_UISetup[] = "UI Settings";
const PROGMEM char Str_SetDefaultBrightness[] = "Save brightness";
const PROGMEM char Str_SetTimeHrs[] = "Set time hours";
const PROGMEM char Str_SetTimeMins[] = "Set time minutes";
const PROGMEM char Str_DelayMeter[] = "Measure delay/ROF";
const PROGMEM char Str_NextPage[] = "Next page >>";

const PROGMEM char Str_BatterySetup[] = "Battery type";
const PROGMEM char Str_FactoryReset[] = "Factory reset";
const PROGMEM char Str_AmmoBarDir[] = "Ammo bar dir. ";
const PROGMEM char Str_ToggleEMFlash[] = "Ammo bar flash";
const PROGMEM char Str_ToggleCenterBtnReload[] = "Sel Btn Reload";
const PROGMEM char Str_CalibrateMag[] = "Calibrate compass";
const PROGMEM char Str_CompassSettings[] = "Compass Mode";

const PROGMEM char Str_EnableHA[] = "Toggle HAssist";
const PROGMEM char Str_ZeroHA[] = "Set HAst. Zero";
const PROGMEM char Str_SetClockCalibration[] = "Clock calibration";
const PROGMEM char Str_ScreenSaverTimeout[] = "ScreenSavr. timeout";


const PROGMEM char* const SettingsLabels[] = {Str_ExitSettings, Str_ManagePresets, Str_SetTimeHrs,Str_SetTimeMins,Str_UISetup,Str_DelayMeter,Str_SetDefaultBrightness,Str_NextPage};
const PROGMEM char* const Settings2Labels[] = {Str_Back,Str_BatterySetup, Str_AmmoBarDir,Str_ToggleEMFlash,Str_ToggleCenterBtnReload,Str_CalibrateMag, Str_CompassSettings, Str_NextPage};
const PROGMEM char* const Settings3Labels[] = {Str_Back, Str_EnableHA,Str_ZeroHA, Str_SetClockCalibration,  Str_FactoryReset};

const PROGMEM char _Str_p1[] = "Preset 1";
const PROGMEM char _Str_p2[] = "Preset 2";
const PROGMEM char _Str_p3[] = "Preset 3";
const PROGMEM char _Str_p4[] = "Preset 4";
const PROGMEM char _Str_p5[] = "Preset 5";
const PROGMEM char _Str_p6[] = "Preset 6";
const PROGMEM char* const PresetLabels[] = {Str_Back, _Str_p1, _Str_p2,_Str_p3,_Str_p4,_Str_p5,_Str_p6};

const PROGMEM char Str_red[] = "Red";
const PROGMEM char Str_green[] = "Green";
const PROGMEM char Str_blue[] = "Blue";
const PROGMEM char* const ColorEditorLabels[] = {Str_Back, Str_red, Str_green, Str_blue};

/*
const PROGMEM char Str_RLM_AUTO_OR_MANUAL[] = "Auto OR Manual";
const PROGMEM char Str_RLM_AUTO_AND_MANUAL[] = "Auto AND Manual";
const PROGMEM char Str_RLM_AUTO_ONLY[] = "Auto only";
const PROGMEM char Str_RLM_MANUAL_ONLY[] = "Manual only";
const PROGMEM char Str_RLM_INVERTED[] = "Inverted";
const PROGMEM char* const ReloadModeLabels[] = {Str_RLM_AUTO_OR_MANUAL,Str_RLM_AUTO_AND_MANUAL,Str_RLM_AUTO_ONLY,Str_RLM_MANUAL_ONLY,Str_RLM_INVERTED};*/

const PROGMEM char Str_UIColor[] = "Base UI Color";
const PROGMEM char Str_BGColor[] = "Background color";
const PROGMEM char Str_StatusColor1[] = "\"Good\" color";
const PROGMEM char Str_StatusColor2[] = "\"Warn\" color";
const PROGMEM char Str_StatusColor3[] = "\"Critical\" color";
const PROGMEM char Str_RotateUI[] = "Rotate UI 90 dgrs";
const PROGMEM char Str_AmmoBarStyle[] = "Ammo Bar Style";
const PROGMEM char* const UISetupLabels[] = {Str_Back,Str_RotateUI, Str_UIColor,Str_BGColor,Str_StatusColor1,Str_StatusColor2,Str_StatusColor3, Str_AmmoBarStyle};


const PROGMEM char Str_BT_RAW_VOLTAGE[] = "Voltage readout";
const PROGMEM char Str_BT_NIMH_7CELL[] = "NiMH 8.4v";
const PROGMEM char Str_BT_NIMH_8CELL[] = "NiMH 9.6v";
const PROGMEM char Str_BT_LIPO_7V4[] = "LiPo 7.4v";
const PROGMEM char Str_BT_LIPO_11V1[] = "LiPo 11.1v";
const PROGMEM char Str_BT_StA_9VOLT[] = "Standalone 9v";
const PROGMEM char Str_BT_Calibrate[] = "  < Calibrate >";
const PROGMEM char* const BTLabels[] = {Str_BT_RAW_VOLTAGE,Str_BT_NIMH_7CELL,Str_BT_NIMH_8CELL,Str_BT_LIPO_7V4,Str_BT_LIPO_11V1,Str_BT_StA_9VOLT, /*Str_BT_Calibrate*/};


const PROGMEM char Str_AB_Solid[] = "Solid";
const PROGMEM char Str_AB_Bullets[] = "Bullets";
const PROGMEM char Str_AB_Bar[] = "Bar";
const PROGMEM char Str_AB_Arc[] = "Arc";
const PROGMEM char Str_AB_Slope[] = "Slope";
const PROGMEM char* const ABLabels[] = {Str_AB_Solid,Str_AB_Bullets, Str_AB_Bar, Str_AB_Arc/*, Str_AB_Slope*/};


const PROGMEM char Str_Off[] = "OFF";

const PROGMEM char Str_HA_IndOnly[] = "Indicator Only";
const PROGMEM char Str_HA_DirOnly[] = "Direction Only";
const PROGMEM char Str_HA_Both[] = "Indicator & Dir.";
const PROGMEM char* const HASettings[] = {Str_Off,Str_HA_IndOnly, Str_HA_DirOnly, Str_HA_Both};

const PROGMEM char Str_CM_Bearing[] = "Bearing";
const PROGMEM char Str_CM_Cardinal[] = "Cardinals (8)";
const PROGMEM char Str_CM_CardinalExt[] = "Cardinals (16)";
const PROGMEM char* const CMSettings[] = {Str_Off,Str_CM_Bearing, Str_CM_Cardinal, Str_CM_CardinalExt};

const PROGMEM char Str_CARD_N[] = " N ";
const PROGMEM char Str_CARD_E[] = " E ";
const PROGMEM char Str_CARD_S[] = " S ";
const PROGMEM char Str_CARD_W[] = " W ";

const PROGMEM char Str_CARD_NE[] = " NE";
const PROGMEM char Str_CARD_SE[] = " SE";
const PROGMEM char Str_CARD_SW[] = "SW ";
const PROGMEM char Str_CARD_NW[] = "NW ";

const PROGMEM char Str_CARD_NNE[] = "NNE";
const PROGMEM char Str_CARD_ENE[] = "ENE";
const PROGMEM char Str_CARD_ESE[] = "ESE";
const PROGMEM char Str_CARD_SSE[] = "SSE";
const PROGMEM char Str_CARD_SSW[] = "SSW";
const PROGMEM char Str_CARD_WSW[] = "WSW";
const PROGMEM char Str_CARD_WNW[] = "WNW";
const PROGMEM char Str_CARD_NNW[] = "NNW";

const PROGMEM char* const CardinalDirections[] = {Str_CARD_N,Str_CARD_NE, Str_CARD_E,Str_CARD_SE,Str_CARD_S,Str_CARD_SW,Str_CARD_W,Str_CARD_NW};
const PROGMEM char* const CardinalDirectionsExt[] = {Str_CARD_N, Str_CARD_NNE, Str_CARD_NE,Str_CARD_ENE, Str_CARD_E, Str_CARD_ESE, Str_CARD_SE, Str_CARD_SSE, Str_CARD_S, Str_CARD_SSW, Str_CARD_SW, Str_CARD_WSW, Str_CARD_W, Str_CARD_WNW, Str_CARD_NW, Str_CARD_NNW};

#endif /* STRINGS_H_ */