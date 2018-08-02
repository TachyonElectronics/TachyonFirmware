/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

STRINGS File (English)
*/


#ifndef STRINGS_H_
#define STRINGS_H_

//PROGMEM strings
const PROGMEM char Str_Version[] = "x0.1";
const PROGMEM char Str_DevMsg[] = "Developed by Martin\nHrehor (c) 2018\n\nPress CENTER to close";

const PROGMEM char Str_Back[] = "<< Back";

const PROGMEM char Str_ExitSettings[] = "<< Save & Exit";
const PROGMEM char Str_ManagePresets[] = "Manage Presets";
const PROGMEM char Str_UISetup[] = "UI Settings";
const PROGMEM char Str_ReloadSetup[] = "Reload behavior";
const PROGMEM char Str_SetDefaultBrightness[] = "Save brightness";
const PROGMEM char Str_SetTime[] = "Set time";
const PROGMEM char Str_NextPage[] = "Next page";

const PROGMEM char Str_BatterySetup[] = "Battery setup";
const PROGMEM char Str_CountDir[] = "Count direction";
const PROGMEM char Str_FactoryReset[] = "Factory reset";
const PROGMEM char Str_SysInfo[] = "System Information";

const PROGMEM char* const SettingsLabels[] = {Str_ExitSettings, Str_ManagePresets, Str_SetTime,Str_UISetup,Str_BatterySetup,Str_SetDefaultBrightness,Str_NextPage};
const PROGMEM char* const Settings2Labels[] = {Str_Back, Str_FactoryReset};

const PROGMEM char _Str_p1[] = "1:";
const PROGMEM char _Str_p2[] = "2:";
const PROGMEM char _Str_p3[] = "3:";
const PROGMEM char _Str_p4[] = "4:";
const PROGMEM char _Str_p5[] = "5:";
const PROGMEM char _Str_p6[] = "6:";
const PROGMEM char* const PresetLabels[] = {Str_Back, _Str_p1, _Str_p2,_Str_p3,_Str_p4,_Str_p5,_Str_p6};

/*
const PROGMEM char Str_RLM_AUTO_OR_MANUAL[] = "Auto OR Manual";
const PROGMEM char Str_RLM_AUTO_AND_MANUAL[] = "Auto AND Manual";
const PROGMEM char Str_RLM_AUTO_ONLY[] = "Auto only";
const PROGMEM char Str_RLM_MANUAL_ONLY[] = "Manual only";
const PROGMEM char Str_RLM_INVERTED[] = "Inverted";
const PROGMEM char* const ReloadModeLabels[] = {Str_RLM_AUTO_OR_MANUAL,Str_RLM_AUTO_AND_MANUAL,Str_RLM_AUTO_ONLY,Str_RLM_MANUAL_ONLY,Str_RLM_INVERTED};*/

const PROGMEM char Str_UIColor[] = "UI Color";
const PROGMEM char Str_BGColor[] = "Background color";
const PROGMEM char Str_CtrColor1[] = "Ammo color (Full)";
const PROGMEM char Str_CtrColor2[] = "Ammo color (2/3)";
const PROGMEM char Str_CtrColor3[] = "Ammo color (1/3)";
const PROGMEM char Str_RotateUI[] = "Rotate UI 90 dgrs";
const PROGMEM char* const UISetupLabels[] = {Str_Back,Str_RotateUI, Str_UIColor,Str_BGColor,Str_CtrColor1,Str_CtrColor2,Str_CtrColor3};

const PROGMEM char Str_BT_RAW_VOLTAGE[] = "Raw voltage";
const PROGMEM char Str_BT_NIMH_7CELL[] = "NiMH 8.4v 7-cell";
const PROGMEM char Str_BT_NIMH_8CELL[] = "NiMH 9.6v 8-cell";
const PROGMEM char Str_BT_LIPO_7V4[] = "LiPo 7.4v";
const PROGMEM char Str_BT_LIPO_11V1[] = "LiPo 11.4v";
const PROGMEM char Str_BT_StA_9VOLT[] = "Standalone 9v";
const PROGMEM char* const BTLabels[] = {Str_BT_RAW_VOLTAGE,Str_BT_NIMH_7CELL,Str_BT_NIMH_8CELL,Str_BT_LIPO_7V4,Str_BT_LIPO_11V1,Str_BT_StA_9VOLT};

#endif /* STRINGS_H_ */