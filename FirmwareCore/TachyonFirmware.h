#ifndef TACHYONFIRMWARE_H_
#define TACHYONFIRMWARE_H_
#include <Wire.h>
#include "../Display/Display.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include "../Bitmaps.h"
#include "../Conf.h"
#include "../RTC.h"
#include "../Strings.h"
#include "../Pins.h"
#include "Macros.h"

#ifdef HAS_MAGNETOMETER
#include "../Magnetometer/LSM303AGR.h"
#endif

DECLARE_BUTTON(BTN_LEFT)
DECLARE_BUTTON(BTN_CENTER)
DECLARE_BUTTON(BTN_RIGHT)
DECLARE_BUTTON(BTN_RLD)

struct __TachyonSettings
{
	uint16_t uiColor, bgColor, statusColors[3];
	int16_t presets[6];
	uint8_t reloadMode;
	uint8_t rotation;
	uint8_t batteryType;
	uint8_t batteryCalibration;
	bool ammoBarDir;
	uint8_t selectedAmmoBar;
	bool emptyFlash;
	bool centerBtnReload;
	int clockCalibration;
	#ifdef HAS_MAGNETOMETER
	Vector3f magOffset;
	bool hAssistEnabled;
	uint8_t compassMode;
	Vector3f hAssistBaseRight;
	#endif // HAS_MAGNETOMETER
};


//Functions
void displayMainScreen(),
loadSettings(),
updateAmmoBar(),
//Size: 128 = 100%
drawAmmoBar(uint8_t size),
updateBrightnessIcon(),
updateCurrentPreset(),
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
//openSimpleListScreen(UIList* _list,char* title,uint8_t _screen,uint8_t _x = 8,uint8_t _y = 12),
openMenuListScreen(uint8_t screen, uint8_t selectedItem = 0),
switchSelectedListItems(uint8_t deselectedItem, uint8_t selectedItem),
editValue(int* value, int _min, int _max),
changeEditedValue(),
openDelayMeter(),
menuSelect(uint8_t item);
inline void stopEditingValue();

uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset);
//inline void soft_reset();
inline void printTime(),
openTimeSetup(),
// openSettingsScreen(),
// openSettingsScreen2(),
updateAmmoCount(),
setBrightness(uint8_t),
updateTime();

#ifdef HAS_MAGNETOMETER
void updateHAssist(),
updateCompass(),
zeroHA();
#endif

#endif /* TACHYONFIRMWARE_H_ */