/* 
	Editor: https://www.visualmicro.com/
			visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
			the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
			all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
			note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: ATmega328PB Crystal Clock, Platform=avr, Package=m328pb
*/

#define __AVR_ATmega328pb__
#define __AVR_ATmega328PB__
#define ARDUINO 10805
#define ARDUINO_MAIN
#define F_CPU 20000000L
#define __AVR__
#define F_CPU 20000000L
#define ARDUINO 10805
#define ARDUINO_AVR_UNO
#define ARDUINO_ARCH_AVR
//
//
void buttonPressed(uint8_t button);
void buttonHeld(uint8_t button);
void reloadInterrupt();
void fire();
void fire_delayMeter();
void reload();
void menuSelect(uint8_t item);
void openMenuListScreen(uint8_t screen, uint8_t selectedItem);
void printTime();
void switchSelectedListItems(uint8_t deselectedItem, uint8_t selectedItem);
void openDelayMeter();
void displayMainScreen();
void updateAmmoBar();
void drawAmmoBar(uint8_t size);
void setBrightness(uint8_t newBrightness);
void updateBrightnessIcon();
void updateTime();
void updateBattery();
void updateCurrentPreset();
void updateHAssist();
void updateCompass();
uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset);
void editValue(int *value, int _min, int _max);
void changeEditedValue();
inline void stopEditingValue();
void zeroHA();
void calibrateBattery();
void loadSettings();
void saveSettings();
void factoryReset();

#include "pins_arduino.h" 
#include "arduino.h"
#include "TachyonFirmware.ino"
