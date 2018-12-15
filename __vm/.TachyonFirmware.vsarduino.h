/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Tachyon V1, Platform=avr, Package=arduino
*/

#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define ARDUINO 10805
#define ARDUINO_MAIN
#define F_CPU 20000000L
#define __AVR__
#define F_CPU 20000000L
#define ARDUINO 10805
#define ARDUINO_TACHYON_V1
#define ARDUINO_ARCH_AVR

//
//
void buttonPressed(uint8_t button);
void buttonHeld(uint8_t button);
void reloadInterrupt();
void fire();
void reload();
void onSettings1Select(uint8_t item);
void onSettings2Select(uint8_t selection);
void onUISetupSelect(uint8_t item);
void onEditPreset(uint8_t preset);
void onChangePresetVal(int16_t value);
void onAcceptPreset(int16_t value);
void openColorEditor(uint16_t* target);
void updateColorEditorSelection();
void updateTestColor(int16_t value);
void acceptColorChannel(int16_t _color_UNUSED);
void openSimpleListScreen(UIList* _list,char* title,uint8_t _screen,uint8_t _x,uint8_t _y);
void openSettingsScreen();
void openSettingsScreen2();
void openTimeSetup();
void printTime();
void displayMainScreen();
void updateAmmoBar();
void drawAmmoBar(uint8_t size);
void setBrightness(uint8_t newBrightness);
void showBrightnessBar();
void updateBrightnessBar();
void hideBrightnessBar();
void updateTime();
void updateBattery();
void updateCurrentPreset();
uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset);
void loadSettings();
void saveSettings();
void factoryReset();

#include "pins_arduino.h" 
#include "arduino.h"
#include "TachyonFirmware.ino"
