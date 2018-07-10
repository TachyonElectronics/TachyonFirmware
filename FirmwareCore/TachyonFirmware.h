#ifndef TACHYONFIRMWARE_H_
#define TACHYONFIRMWARE_H_
#include <Wire.h>
#include "../Adafruit-ST7735/Adafruit_ST7735.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include "../Bitmaps.h"
#include "../Conf.h"
#include "../UI.h"
#include "../RTC.h"
#include "../Strings.h"
#include "Macros.h"

DECLARE_BUTTON(BTN_LEFT)
DECLARE_BUTTON(BTN_CENTER)
DECLARE_BUTTON(BTN_RIGHT)
DECLARE_BUTTON(BTN_RLD)

struct __TachyonSettings
{
	uint16_t uiColor, bgColor, ctrColor1, ctrColor2, ctrColor3;
	int16_t presets[6];
	uint8_t reloadMode;
	uint8_t rotation;
	uint8_t batteryType;
	bool counterDirection;
} settings;

//Variables
uint8_t updateAmmoNextLoop;
uint8_t currentScreen;
UIElement* focusedUiElement;
Adafruit_ST7735 disp = Adafruit_ST7735(DSP_CS,11,DSP_RST);
unsigned long currentMillis,incrementMillis, hideBrightnessMillis, centerBtnMillis, battMeasurementMillis;
unsigned long deltaMillis;
RTC Rtc = RTC();
//bool queuedTimeSave;
uint8_t currentPreset, changedPreset;
int16_t ammo;
uint16_t ammoColor;
uint8_t brightness;
uint16_t* editedColor;
uint16_t emptyFlashCtr = 0;
bool emptyFlashState = 0;
bool magOut;
bool changingPreset;
bool mainCenterBtnReleased;
bool adjustingBrightness;
bool queuedTimeSave;
bool fireDebounce = 0;

//Functions
void displayMainScreen(),
loadSettings(),
updateAmmoBar(),
showBrightnessBar(),
updateBrightnessBar(),
updateCurrentPreset(),
hideBrightnessBar(),
updateBattery(),
openPresetList(),
buttonPressed(uint8_t),
buttonHeld(uint8_t),
reload(),
saveSettings(),
factoryReset(),
fire(),
executeCommand(uint8_t cmd),
reloadInterrupt();
uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset);

//inline void soft_reset();
inline void printTime(),
openTimeSetup(),
openSettingsScreen(),
openSettingsScreen2(),
updateAmmoCount(),
openColorEditor(uint16_t* target),
setBrightness(uint8_t),
updateColorEditorSelection();


//=======================UI========================
#pragma region UI
inline void onSettings1Select(uint8_t);
UIList settings1 = UIList(&disp,&onSettings1Select,112,12,4,&settings.uiColor,&settings.bgColor,SettingsLabels,7);

inline void onEditPreset(uint8_t);
UIList presetList = UIList(&disp,&onEditPreset,66,12,4,&settings.uiColor,&settings.bgColor,PresetLabels,7);
inline void onAcceptPreset(uint16_t value), onChangePresetVal(uint16_t value);
UIInvisibleSlider presetSlider = UIInvisibleSlider(&disp,&onAcceptPreset,&onChangePresetVal,0,999,50);

//UIList reloadModeList = UIList(&disp,[settings](uint8_t val){settings.reloadMode = val;openSettingsScreen();},102,12,4,&settings.uiColor,&settings.bgColor,ReloadModeLabels,5);

inline void onUISetupSelect(uint8_t);
UIList uiSetupList = UIList(&disp,&onUISetupSelect,112,12,4,&settings.uiColor,&settings.bgColor,UISetupLabels,7);

uint8_t selectedColorChannel;
uint16_t testColor;
inline void updateTestColor(int16_t);
inline void selectColorChannel(uint8_t channel);
inline void acceptColorChannel(int16_t);
UIInvisibleList colorEditorMenu = UIInvisibleList(&disp,&selectColorChannel,4);
const uint16_t _const_red = ST7735_RED, _const_green = ST7735_GREEN, _const_blue = ST7735_BLUE;
UIVerticalSlider colorSliderR = UIVerticalSlider(&disp,10,104,&_const_red,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,31,50);
UIVerticalSlider colorSliderG = UIVerticalSlider(&disp,10,104,&_const_green,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,63,25);
UIVerticalSlider colorSliderB = UIVerticalSlider(&disp,10,104,&_const_blue,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,31,50);

inline void acceptTime();
UIInvisibleSlider minuteSlider = UIInvisibleSlider(&disp,&acceptTime,[disp, Rtc, minuteSlider](){Rtc.minutes = minuteSlider.value; disp.setCursor(50,50); printTime();},0,59,250);
UIInvisibleSlider hourSlider = UIInvisibleSlider(&disp,[minuteSlider, Rtc, settings, disp](){focusedUiElement = &minuteSlider; minuteSlider.value = Rtc.minutes; disp.drawFastHLine(50,62,12,settings.bgColor);disp.drawFastHLine(68,62,12,settings.uiColor);},[disp, Rtc, settings, hourSlider](){Rtc.hours = hourSlider.value; disp.setCursor(50,50); printTime();},0,23,400);

inline void onSettings2Select(uint8_t);

UIList settings2 = UIList(&disp,&onSettings2Select,112,12,4,&settings.uiColor,&settings.bgColor,Settings2Labels,2);

UIList btList = UIList(&disp,[settings](uint8_t val){settings.batteryType = val;openSettingsScreen();},102,12,4,&settings.uiColor,&settings.bgColor,BTLabels,6);
#pragma endregion
//=================================================

#endif /* TACHYONFIRMWARE_H_ */