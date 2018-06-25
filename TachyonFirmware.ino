/*
	OFFICIAL TACHYON FIRMWARE
	Version 1.x
	
	(c) Martin Hrehor 2018

*/

#include <Wire.h>
#include "Adafruit-ST7735/Adafruit_ST7735.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include "Bitmaps.h"
#include "Conf.h"
#include "UI.h"
#include "RTC.h"

#define SUP b.updating = 1
#define EUP b.updating = 0
#define UP b.updating

uint16_t buttonCtrs[_BTN_COUNT];
#define DECLARE_BUTTON(name) bool name##_isPressed; uint16_t _##name##_debounceCtr;
#define HANDLE_BUTTON_INTERRUPT(name, port, pin) if(bit_is_clear(port,pin)) { if(!name##_isPressed && _##name##_debounceCtr >= BUTTON_DEBOUNCE_INTERVAL) {buttonPressed(name);name##_isPressed=1;buttonCtrs[name]=0; _##name##_debounceCtr = 0; } } else if(name##_isPressed){name##_isPressed = 0; _##name##_debounceCtr = 0;}
#define HANDLE_BUTTON_LOOP(name) if(name##_isPressed){buttonCtrs[name] += deltaMillis; buttonHeld(name);}  if(_##name##_debounceCtr < BUTTON_DEBOUNCE_INTERVAL)_##name##_debounceCtr += deltaMillis;

#define SHIFT_RED 11
#define SHIFT_GREEN 5
#define SHIFT_BLUE 0
#define GET_RED(color) ((color & 0xF800) >> 11)
#define GET_GREEN(color) ((color & 0x7E0) >> 5)
#define GET_BLUE(color) (color & 0x1F)

DECLARE_BUTTON(BTN_LEFT)
DECLARE_BUTTON(BTN_CENTER)
DECLARE_BUTTON(BTN_RIGHT)
DECLARE_BUTTON(BTN_RLD)

//MOSI = 11; SCK = 13

//Buttons: L=PC1	C=PC2	R=PC3
uint8_t currentScreen;
UIElement* focusedUiElement;
bool spiTransferStatus;
Adafruit_ST7735 disp = Adafruit_ST7735(DSP_CS,11,&spiTransferStatus,DSP_RST);
unsigned long currentMillis,incrementMillis, hideBrightnessMillis, centerBtnMillis, battMeasurementMillis;
unsigned long deltaMillis;
struct
{
	uint8_t updating:1;
	uint8_t magOut:1;
	uint8_t changingPreset:1;
	uint8_t mainCenterBtnReleased:1;
	uint8_t adjustingBrightness:1;
} b;
struct
{
	uint16_t uiColor, bgColor, ctrColor1, ctrColor2, ctrColor3;
	int16_t presets[6];
	uint8_t reloadMode;
} settings;

RTC Rtc = RTC();
bool updatingAmmo, queuedUpdate, queuedTimeSave;
uint8_t currentPreset, changedPreset;
int16_t ammo;

uint16_t ammoColor;

uint8_t brightness;

uint16_t* editedColor;

void displayMainScreen(),
updateAmmoBar(),
updateAmmoCount(),
showBrightnessBar(),
updateBrightnessBar(),
updateCurrentPreset(),
hideBrightnessBar(),
updateBattery(),
//openSettingsScreen(),
openSysInfoScreen(),
openPresetList(),
buttonPressed(uint8_t),
buttonHeld(uint8_t),
reload(),
loadSettings(),
saveSettings(),
factoryReset(),
fire(),
executeCommand(uint8_t cmd),
reloadInterrupt();

//inline void soft_reset();
inline void printTime();
inline void openTimeSetup();
inline void openSettingsScreen();
inline void updateAmmo();
inline void openColorEditor(uint16_t* target);
//=======================UI========================
void onList1Select(uint8_t);
UIList list1 = UIList(&disp,&onList1Select,112,12,4,&settings.uiColor,&settings.bgColor,List1Contents,7);

void onEditPreset(uint8_t);
UIList presetList = UIList(&disp,&onEditPreset,66,12,4,&settings.uiColor,&settings.bgColor,PresetLabels,7);
void onAcceptPreset(uint16_t value), onChangePresetVal(uint16_t value);
UIInvisibleSlider presetSlider = UIInvisibleSlider(&disp,&onAcceptPreset,&onChangePresetVal,0,999,50);

UIList reloadModeList = UIList(&disp,[settings](uint8_t val){settings.reloadMode = val;openSettingsScreen();},102,12,4,&settings.uiColor,&settings.bgColor,ReloadModeLabels,5);

void onUISetupSelect(uint8_t);
UIList uiSetupList = UIList(&disp,&onUISetupSelect,112,12,4,&settings.uiColor,&settings.bgColor,UISetupLabels,6);

uint8_t selectedColorChannel;
uint16_t testColor;
void updateTestColor(int16_t);
void selectColorChannel(uint8_t channel);
void acceptColorChannel(int16_t);
UIInvisibleList colorEditorMenu = UIInvisibleList(&disp,&selectColorChannel,4);
const uint16_t _const_red = ST7735_RED, _const_green = ST7735_GREEN, _const_blue = ST7735_BLUE;
UIVerticalSlider colorSliderR = UIVerticalSlider(&disp,10,104,&_const_red,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,31,50);
UIVerticalSlider colorSliderG = UIVerticalSlider(&disp,10,104,&_const_green,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,63,25);
UIVerticalSlider colorSliderB = UIVerticalSlider(&disp,10,104,&_const_blue,&settings.bgColor,&acceptColorChannel,&updateTestColor,0,31,50);

void acceptTime();
UIInvisibleSlider minuteSlider = UIInvisibleSlider(&disp,&acceptTime,[disp, Rtc, minuteSlider](){Rtc.minutes = minuteSlider.value; disp.setCursor(50,50); printTime();},0,59,250);
UIInvisibleSlider hourSlider = UIInvisibleSlider(&disp,[minuteSlider, Rtc, settings, disp](){focusedUiElement = &minuteSlider; minuteSlider.value = Rtc.minutes; disp.drawFastHLine(50,62,12,settings.bgColor);disp.drawFastHLine(68,62,12,settings.uiColor);},[disp, Rtc, settings, hourSlider](){Rtc.hours = hourSlider.value; disp.setCursor(50,50); printTime();},0,23,400);


#define openSimpleListScreen(_list, title, _screen) currentScreen = _screen; disp.fillScreen(settings.bgColor); disp.setTextColor(settings.uiColor); disp.setCursor(40,0); disp.print(title); _list.draw(8,12); focusedUiElement = &_list;
#define openSimpleListScreen_xy(_list, title, _screen,_x,_y) currentScreen = _screen; disp.fillScreen(settings.bgColor); disp.setTextColor(settings.uiColor); disp.setCursor(40,0); disp.print(title); _list.draw(_x,_y); focusedUiElement = &_list;
//=================================================




inline void setBrightness(uint8_t);
inline void updateColorEditorSelection();


void setup()
{
	loadSettings();
	//Setup ADC
	/*ADMUX = 0b11000111; //Setup adc ref voltage, left adjust and select channel
	ADCSRA = 0b00001110; //Enable ADC interrupt, setup prescaler to divide by 64*/
	analogReference(INTERNAL);
	
	pinMode(DSP_BL, OUTPUT);

	
	//Set MOSI and SCK as output (we will need to use them manually for display D/C transmission
	DDRB |= ((1 << PINB3) | (1 << PINB5));
	spiTransferStatus = 0;
	
	analogWrite(DSP_BL, 0); //Turn off display until initialization is comlete
	disp.initR(INITR_144GREENTAB);
	delay(100);
	//	disp.fillScreen(ST7735_RED);
	SPSR |= 1 << SPI2X;
	//SPCR = 0b11010000;
	SPCR &= ~0b00000011;
	
	disp.fillScreen(settings.bgColor);
	disp.drawFastBitmap(14,14,Logo,settings.uiColor,settings.bgColor);
	setBrightness(brightness);
	delay(250);
	
	//Setup pin change interrupt
	PCICR |= _BV(PCIE1);// | _BV(PCIE2); //Enable pc interrupt 1 (pins PCINT8-14)
	PCMSK1 |= _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10); //Mask PC interrupt pins so inly buttons are enabled
	//PCMSK2 |= _BV(PCINT17);
	


	ammo = settings.presets[currentPreset];


	PORTC |= (1 << PINC0) | (1 << PINC1) | (1 << PINC2); //pull up side buttons
	//PORTD |= _BV(RLD);
	

	attachInterrupt(1,&fire,CHANGE);
	PORTD |= _BV(PD2);
	attachInterrupt(0, &reloadInterrupt, CHANGE);
	Wire.begin();
	Rtc.Init();
	Serial.begin(9600);
	disp.fillScreen(settings.bgColor);
	displayMainScreen();
	updateAmmoBar();
	updateAmmoCount();
	sei();
}
void reloadInterrupt()
{
	HANDLE_BUTTON_INTERRUPT(BTN_RLD,PIND,PIND2);
}
ISR(PCINT1_vect)
{
	HANDLE_BUTTON_INTERRUPT(BTN_CENTER,PINC,PINC1);
	HANDLE_BUTTON_INTERRUPT(BTN_LEFT,PINC,PINC0);
	HANDLE_BUTTON_INTERRUPT(BTN_RIGHT,PINC,PINC2);
}
ISR(PCINT2_vect)
{
	//	HANDLE_BUTTON_INTERRUPT(BTN_RLD,PIND,RLD);
}
bool fireDebounce = 0;
void fire()
{
	cli();
	if(bit_is_clear(PIND,PIND3))
	{
		if(!fireDebounce)
		{
			ammo--;
			queuedUpdate = 1;
			fireDebounce = 1;
		}
	}
	else
	{
		fireDebounce = 0;
	}
	sei();
}
void updateAmmo()
{
	updateAmmoBar();
	updateAmmoCount();
	//updatingAmmo = 0;
}
void loop()
{
	deltaMillis = millis() - currentMillis;
	currentMillis += deltaMillis;
	
	if(Serial.available())
	executeCommand(Serial.read());
	
	if(queuedUpdate)
	{
		updateAmmo();
		queuedUpdate = 0;
	}
	
	//Brightness adjustment
	if(currentScreen == SCREEN_MAIN){
		
		//Center button at main screen
		if(!BTN_CENTER_isPressed && !b.mainCenterBtnReleased)
		{
			b.mainCenterBtnReleased = 1;
			updateCurrentPreset();
			centerBtnMillis = millis();
		}
		if(currentMillis >  centerBtnMillis + CENTER_HOLD_TIME)
		{
			if(b.mainCenterBtnReleased)
			{
				b.changingPreset = 0;
				b.mainCenterBtnReleased = 0;
				if(changedPreset != currentPreset)
				{
					currentPreset = changedPreset;
					reload();
				}
			}
			else if(BTN_CENTER_isPressed)
			{
				openSettingsScreen();
				//openSimpleListScreen(list1,"Settings",SCREEN_SETTINGS);
			}
		}
		
		if(!b.changingPreset)
		{
			if(BTN_RIGHT_isPressed)
			{
				if(currentMillis > incrementMillis + INCREMENT_TIME)
				{
					incrementMillis = millis();
					setBrightness(brightness < 255? brightness+1 : 255);
					hideBrightnessMillis = millis();
					if(!b.adjustingBrightness){
						b.adjustingBrightness = 1;
						showBrightnessBar();
					}
					updateBrightnessBar();
				}
			}
			else if(BTN_LEFT_isPressed)
			{
				if(currentMillis > incrementMillis + INCREMENT_TIME)
				{
					incrementMillis = millis();
					setBrightness(brightness > MIN_BRIGHTNESS? brightness-1  : MIN_BRIGHTNESS);
					hideBrightnessMillis = millis();
					if(!b.adjustingBrightness){
						b.adjustingBrightness = 1;
						showBrightnessBar();
					}
					updateBrightnessBar();
				}
			}
			else if(b.adjustingBrightness && currentMillis > hideBrightnessMillis + BRIGHTNESS_STAY_TIME){
				hideBrightnessBar();
				b.adjustingBrightness = 0;
			}
		}
	}
	
	HANDLE_BUTTON_LOOP(BTN_LEFT);
	HANDLE_BUTTON_LOOP(BTN_RIGHT);
	HANDLE_BUTTON_LOOP(BTN_CENTER);
	HANDLE_BUTTON_LOOP(BTN_RLD)
	
	//Battery measurement
	if(currentScreen == SCREEN_MAIN && currentMillis > battMeasurementMillis + BATT_MEASURMENT_INTERVAL){
		updateBattery();
		disp.setCursor(0,0);
		if(queuedTimeSave)
		{
			disp.print("FAIL");
			Rtc.SetTime();
			queuedTimeSave = 0;
		}else
		Rtc.Update();
		disp.setCursor(0,0);
		printTime();
	}
	
	if(currentScreen == SCREEN_COLOR)
	if(selectedColorChannel != colorEditorMenu.selectedItem) updateColorEditorSelection();
	
}
void reload(){
	ammo = settings.presets[currentPreset];
	if(currentScreen == SCREEN_MAIN){
		disp.fillRect(1,49,126,48,settings.bgColor);
		updateAmmoBar();
		updateAmmoCount();
	}
}


//==============================Main screen==============================

void displayMainScreen()
{
	focusedUiElement = nullptr;
	currentScreen = SCREEN_MAIN;
	disp.fillScreen(settings.bgColor);
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(95,13);
	disp.print("P:");
	disp.drawFastBitmap(88,3,IconBattery,settings.uiColor,settings.bgColor);
	updateAmmoBar();
	updateAmmoCount();
	updateCurrentPreset();
}
void updateAmmoBar()
{
	uint8_t crop = ammo > 0 ? 64-map(ammo,0,settings.presets[currentPreset],0,64) : 0;

	
	if(crop > 42) ammoColor = settings.ctrColor3;
	else if(crop > 21) ammoColor = settings.ctrColor2;
	else ammoColor = settings.ctrColor1;
	disp.drawFastCroppedBitmap(0,112,AmmoBar1L,ammoColor,settings.bgColor,crop,0);
	disp.drawFastCroppedBitmap(64,112,AmmoBar1R,ammoColor,settings.bgColor,-crop,0);
}

void updateAmmoCount()
{
	if(settings.presets[currentPreset] < 100)
	{
		uint8_t _10 = abs(ammo) / 10;
		uint8_t _1 = abs(ammo) - (10 * _10);
		
		if(_1 == 1)
		disp.drawFastBitmap(66,49,C_L1R,ammoColor,settings.bgColor);
		else if (b.magOut)
		disp.drawFastBitmap(66,49,C_Ldash,ammoColor,settings.bgColor);
		else
		disp.drawFastBitmap(66,49,C_Lnums[_1],ammoColor,settings.bgColor);
		
		if(ammo < 0 || b.magOut)
		disp.drawFastBitmap(14,49,C_Ldash,ammoColor,settings.bgColor);
		else
		disp.drawFastBitmap(14,49,C_Lnums[_10],ammoColor,settings.bgColor);
		

	}
	else if(settings.presets[currentPreset] < 1000)
	{
		uint8_t _100 = abs(ammo) / 100;
		uint8_t _10 = (abs(ammo) - _100 * 100) / 10;
		uint8_t _1 = abs(ammo) - (10 * _10) - (_100 * 100);
		
		if(_1 == 1)
		disp.drawFastBitmap(89,54,C_S1R,ammoColor,settings.bgColor);
		else if (b.magOut)
		disp.drawFastBitmap(89,54,C_Sdash,ammoColor,settings.bgColor);
		else
		disp.drawFastBitmap(89,54,C_Snums[_1],ammoColor,settings.bgColor);
		
		if(b.magOut)
		disp.drawFastBitmap(45,54,C_Sdash,ammoColor,settings.bgColor);
		else
		disp.drawFastBitmap(45,54,C_Snums[_10],ammoColor,settings.bgColor);
		
		if(_100 == 1)
		disp.drawFastBitmap(1,54,C_S1L,ammoColor,settings.bgColor);
		else if (b.magOut || ammo < 0)
		disp.drawFastBitmap(1,54,C_Sdash,ammoColor,settings.bgColor);
		else
		disp.drawFastBitmap(1,54,C_Snums[_100],ammoColor,settings.bgColor);
		
		
	}
}
void setBrightness(uint8_t newBrightness)
{
	brightness = newBrightness;
	analogWrite(DSP_BL, brightness);
}

void showBrightnessBar()
{
	disp.drawFastBitmap(2,28,IconBrightness,settings.uiColor,settings.bgColor);
	disp.fillRect(16,29,104,8,settings.uiColor);
}
void updateBrightnessBar()
{
	uint8_t crop =map(brightness,MIN_BRIGHTNESS,255,0,100);
	//disp.fillCroppedRect(17,25,102,6,ST7735_RED,settings.uiColor,100-crop,0); //Bi-color bar
	disp.fillCroppedRect(19,30,100,6,settings.bgColor,settings.uiColor,-crop,0); //Monochrome bar
	
}
void hideBrightnessBar()
{
	disp.fillRect(2,28,BITMAP_SIZE(IconBrightness),settings.bgColor);
	disp.fillRect(16,29,104,8,settings.bgColor);
}
void updateBattery()
{
	uint16_t result = analogRead(6);
	float voltage = (result * 11 / 1024.0f) *1.127f;

	disp.setCursor(102,3);
	disp.setFont();
	disp.setTextSize(1);
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.print(voltage,1);
	battMeasurementMillis = millis();
}
void updateCurrentPreset()
{
	disp.setTextColor(settings.uiColor, settings.bgColor);
	disp.setCursor(105,13);
	disp.setTextSize(2);
	disp.print(b.changingPreset? changedPreset+1 : currentPreset+1);
	disp.setTextSize(1);
	
	disp.setTextColor(b.changingPreset? settings.uiColor : settings.bgColor);
	disp.setCursor(95,22);
	disp.print('<');
	disp.setCursor(119,22);
	disp.print('>');
	
	disp.setTextColor(settings.uiColor,settings.bgColor);
}
//====================================================================

void openSettingsScreen()
{
	currentScreen = SCREEN_SETTINGS;
	disp.fillScreen(settings.bgColor);
	disp.setTextColor(settings.uiColor);
	disp.setCursor(40,0);
	disp.print("Settings");
	list1.draw(8,12);
	focusedUiElement = &list1;
}

//=============
//Settings menu
//=============
void onList1Select(uint8_t item)
{
	switch(item)
	{
		case 0:	//Save and exit
		saveSettings();
		displayMainScreen();
		return;
		
		case 1: //Manage presets
		openSimpleListScreen(presetList,"Presets",SCREEN_PRESETS);
		disp.setTextColor(settings.uiColor,settings.bgColor);
		for (int i = 1; i < presetList.itemCount;i++)
		{
			disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*i);
			if(settings.presets[i-1])disp.print(settings.presets[i-1]);
			else disp.print("OFF");
		}
		return;
		
		case 2: //Set time
		openTimeSetup();
		return;
		
		case 3: //UI Setup
		openSimpleListScreen(uiSetupList,"UI Settings",SCREEN_UISETUP);
		return;
		
		case 4:	//Reload behavior
		reloadModeList.selectedItem = settings.reloadMode;
		openSimpleListScreen_xy(reloadModeList,"Reload Modes",SCREEN_RELOADMODES,18,12);
		disp.setTextColor(settings.uiColor,settings.bgColor);
		disp.setCursor(8,reloadModeList.y+2+(reloadModeList.itemHeight+reloadModeList.spacing)*settings.reloadMode);
		disp.print('>');
		return;
		
		case 5: //Save brightness
		eeprom_busy_wait();
		eeprom_update_byte(EA_BRIGHTNESS,brightness);
		disp.setTextColor(settings.bgColor,settings.uiColor);
		disp.setCursor(105,2+list1.y+(list1.itemHeight+list1.spacing)*5);
		disp.print("OK");
		return;
		
		case 6: //sysinfo
		openSysInfoScreen();
		return;
	}
}

void buttonPressed(uint8_t button)
{
	if(focusedUiElement){
		focusedUiElement->button_pressed(button);
		return;
	}
	
	switch(button)
	{
		case BTN_CENTER:
		switch(currentScreen)
		{
			case SCREEN_MAIN:
			changedPreset = currentPreset;
			b.changingPreset = 1;
			updateCurrentPreset();
			centerBtnMillis = millis();
			break;
			
			case SCREEN_SYSINFO:
			openSettingsScreen();
			break;
		}
		return;
		
		case BTN_LEFT:
		if(currentScreen == SCREEN_MAIN && b.changingPreset){
			do
			{
				if(changedPreset)
				changedPreset--;
				else changedPreset = _PRESET_COUNT-1;
			} while (!settings.presets[changedPreset]);
			updateCurrentPreset();
			centerBtnMillis = millis();
		}
		return;
		
		case BTN_RIGHT:
		if(currentScreen == SCREEN_MAIN && b.changingPreset){
			do
			{
				if(changedPreset < _PRESET_COUNT-1)
				changedPreset++;
				else changedPreset = 0;
			} while (!settings.presets[changedPreset]);
			updateCurrentPreset();
			centerBtnMillis = millis();
		}
		return;
		
		case BTN_RLD:
		reload();
		return;
	}
}
void buttonHeld(uint8_t button)
{
	if(focusedUiElement){
		focusedUiElement->button_held(button,&buttonCtrs[button]);
		return;
	}
}
void openSysInfoScreen()
{
	currentScreen = SCREEN_SYSINFO;
	focusedUiElement = nullptr;
	//disp.drawFastBitmap(0,0,SysInfoScreenBg, settings.uiColor, settings.bgColor);
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(0,32);
	disp.print("FW Version: ");
	disp.println(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&StringList[0]));
	disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&StringList[1]));
}

//==============================Preset edit==============================

void onEditPreset(uint8_t preset)
{
	if(!preset){
		openSettingsScreen();
		return;
	}
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(80,presetList.y+2+(presetList.itemHeight+presetList.spacing)*preset);
	disp.print('>');
	disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
	presetSlider.minValue = (preset-1) ? 0 : 1; //Set min value for preset #1 as 1 -> it musn't be turned off
	focusedUiElement = &presetSlider;
	presetSlider.value = settings.presets[presetList.selectedItem - 1];
	disp.print(presetSlider.value);
	disp.print("  ");
}
void onChangePresetVal(uint16_t value)
{
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
	disp.print(value);
	disp.print(' '); //Print one space to remove any 3-character numbers

}
void onAcceptPreset(uint16_t value)
{
	settings.presets[presetList.selectedItem - 1] = value; //make sure the first preset is never off, otherwise the system will get stuck while searching for available presets
	focusedUiElement = &presetList;
	disp.writeFillRect(80,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem,10,10,settings.bgColor);
	if(!value){
		disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
		disp.print("OFF");
	}
}

//=======================================================================

//============================USER SETTINGS===================================
void loadSettings()
{
	eeprom_busy_wait();
	uint8_t vtag = eeprom_read_byte(0x00);
	if(vtag != _EEPROM_VERIFY_TAG){factoryReset(); return;}
	
	brightness = eeprom_read_byte(EA_BRIGHTNESS);
	
	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		((uint8_t*)(&settings))[i] = eeprom_read_byte(EA_SETTINGS+i);
	}
}
void saveSettings()
{
	eeprom_busy_wait();
	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		eeprom_update_byte(EA_SETTINGS+i,((uint8_t*)(&settings))[i]);
	}
}
void factoryReset()
{
	eeprom_busy_wait();
	eeprom_write_byte(0x00,_EEPROM_VERIFY_TAG);
	eeprom_write_byte(EA_BRIGHTNESS,FACT_BRIGHTNESS);
	
	eeprom_write_word(EA_SETTINGS,FACT_UICOLOR);
	eeprom_write_word(EA_SETTINGS+2,ST7735_BLACK);
	eeprom_write_word(EA_SETTINGS+4,FACT_CTRCOLOR1);
	eeprom_write_word(EA_SETTINGS+6,FACT_CTRCOLOR2);
	eeprom_write_word(EA_SETTINGS+8,FACT_CTRCOLOR3);
	
	eeprom_write_word(EA_SETTINGS+10,FACT_PRESET1);
	eeprom_write_word(EA_SETTINGS+12,FACT_PRESET2);
	//eeprom_write_byte(EA_SETTINGS+16,FACT_RLM);
	size_t s = sizeof(settings);
	for (size_t i = 14; i < s; i++)
	{
		eeprom_write_byte(EA_SETTINGS+i, 0);
	}
	loadSettings();
}
//==============================UI Setup==============================
void onUISetupSelect(uint8_t item)
{
	switch(item)
	{
		case 0:
		openSettingsScreen();
		return;
		
		case 1:
		openColorEditor(&settings.uiColor);
		return;
		case 2:
		openColorEditor(&settings.bgColor);
		return;
		case 3:
		openColorEditor(&settings.ctrColor1);
		return;
		case 4:
		openColorEditor(&settings.ctrColor2);
		return;
		case 5:
		openColorEditor(&settings.ctrColor3);
		return;
	}
}

//====================================================================

//==============================Color editor==========================
void openColorEditor(uint16_t* target)
{
	currentScreen = SCREEN_COLOR;
	selectedColorChannel = 0;
	editedColor = target;
	testColor = *editedColor;
	focusedUiElement = &colorEditorMenu;
	disp.fillScreen(settings.bgColor);
	colorSliderR.value = GET_RED(testColor);
	colorSliderR.draw(8,8);
	colorSliderG.value = GET_GREEN(testColor);
	colorSliderG.draw(24,8);
	colorSliderB.value = GET_BLUE(testColor);
	colorSliderB.draw(40,8);
	
	disp.drawRect(64,100,56,12,settings.uiColor);
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(80,101);
	disp.print("Back");
	disp.setCursor(10,10);
	disp.fillRect(88,16,24,24,testColor);
	disp.drawRect(87,15,26,26,settings.uiColor);
	updateColorEditorSelection();
}
void selectColorChannel(uint8_t channel)
{
	selectedColorChannel = channel;
	switch(selectedColorChannel)
	{
		case 3:
		focusedUiElement = &colorSliderR;
		break;
		
		case 2:
		focusedUiElement = &colorSliderG;
		break;
		
		case 1:
		focusedUiElement = &colorSliderB;
		break;
		
		case 0: //This is the back button
		*editedColor = testColor;
		openSimpleListScreen(uiSetupList,"UI Settings",SCREEN_UISETUP);
		return;
	}
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(43-((selectedColorChannel-1)*16),114);
	disp.print('^');
}
void updateColorEditorSelection()
{
	if(colorEditorMenu.selectedItem == 0)
	{
		disp.fillRect(64,100,56,12,settings.uiColor);
		disp.setTextColor(settings.bgColor,settings.uiColor);
		disp.setCursor(80,102);
		disp.print("Back");
	}
	else
	{
		disp.setTextColor(settings.uiColor,settings.bgColor);
		disp.setCursor(43-((colorEditorMenu.selectedItem-1)*16),114);
		disp.print('o');
	}
	
	if(selectedColorChannel != colorEditorMenu.selectedItem)
	{
		if(selectedColorChannel == 0)
		{
			disp.drawRect(64,100,56,12,settings.uiColor);
			disp.fillRect(65,101,54,10,settings.bgColor);
			disp.setTextColor(settings.uiColor,settings.bgColor);
			disp.setCursor(80,102);
			disp.print("Back");
		}
		else
		{
			disp.fillRect(43-((selectedColorChannel-1)*16),114,12,10,settings.bgColor);
		}
		selectedColorChannel = colorEditorMenu.selectedItem;
	}
}
void updateTestColor(int16_t value)
{
	switch(selectedColorChannel)
	{
		case 3:
		testColor &= ~(ST7735_RED);
		testColor |= value << SHIFT_RED;
		break;
		
		case 2:
		testColor &= ~(ST7735_GREEN);
		testColor |= value << SHIFT_GREEN;
		break;
		
		case 1:
		testColor &= ~(ST7735_BLUE);
		testColor |= value;
		break;
	}
	disp.fillRect(88,16,24,24,testColor);
}
void acceptColorChannel(int16_t _color_UNUSED)
{
	focusedUiElement = &colorEditorMenu;
	disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(43-((selectedColorChannel-1)*16),114);
	disp.print('o');
}
//====================================================================
void openTimeSetup()
{
	focusedUiElement = &hourSlider;
	hourSlider.value = Rtc.hours;
	disp.fillScreen(settings.bgColor);
	disp.setCursor(50,50);
	disp.setTextColor(settings.uiColor,settings.bgColor);
	printTime();
	disp.drawFastHLine(50,62,12,settings.uiColor);
}
void printTime()
{
	if(Rtc.hours < 10)disp.print('0');
	disp.print(Rtc.hours);
	disp.print(':');
	if(Rtc.minutes < 10)disp.print('0');
	disp.print(Rtc.minutes);

}
void acceptTime()
{
	queuedTimeSave = 1;
	openSettingsScreen();
}
/*
void soft_reset()
{
cli();
asm("ijmp" ::"z" (0x0000));//(0x3F00));
}*/

void executeCommand(uint8_t cmd)
{
	switch(cmd)
	{
		case 0x00:
		Serial.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&StringList[0]));
		break;
		
		case 0x01: //GET_TIME
		Serial.write(Rtc.seconds);
		Serial.write(Rtc.minutes);
		Serial.write(Rtc.hours);
		break;
		
		default:
		Serial.println("UnkCmd");
		break;
	}
}