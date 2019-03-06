#ifndef TACHYONFIRMWARE_H_
#define TACHYONFIRMWARE_H_
#include <Wire.h>
#include "../Display/Display.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include "../Bitmaps.h"
#include "../Conf.h"
#include "../UI.h"
#include "../RTC.h"
#include "../Strings.h"
#include "../Pins.h"
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
	bool ammoBarDir;
	uint8_t selectedAmmoBar;
	bool emptyFlash;
	bool centerBtnReload;
	int8_t clockCalibration;
};


//Functions
void displayMainScreen(),
loadSettings(),
updateAmmoBar(),
//Size: 128 = 100%
drawAmmoBar(uint8_t size),
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
reloadInterrupt(),
printCalibration(),
timeSetupSlider_OnValueChange(),
timeSetupSlider_Accept(),
openSimpleListScreen(UIList* _list,char* title,uint8_t _screen,uint8_t _x = 8,uint8_t _y = 12);

uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset);
//inline void soft_reset();
inline void printTime(),
openTimeSetup(),
openSettingsScreen(),
openSettingsScreen2(),
updateAmmoCount(),
openColorEditor(uint16_t* target),
setBrightness(uint8_t),
updateColorEditorSelection(),
updateTime();

#endif /* TACHYONFIRMWARE_H_ */