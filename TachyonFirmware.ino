/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

*/

#include "FirmwareCore/TachyonFirmware.h"
#include "FirmwareCore/Definitions.h"

__TachyonSettings settings;

#pragma region Global Variables

//Variables
uint8_t factoryResetCtr;
uint8_t updateAmmoNextLoop;
uint8_t currentScreen;
UIElement* focusedUiElement;
Adafruit_ST7735 disp = Adafruit_ST7735(DSP_CS,11,DSP_RST);
unsigned long currentMillis,incrementMillis, hideBrightnessMillis, centerBtnMillis, battMeasurementMillis;
unsigned long deltaMillis;
RTC Rtc = RTC();
uint8_t timeSetupStage = 0;
uint8_t currentPreset, changedPreset;
int16_t ammo;
uint16_t ammoColor;
uint8_t brightness;
uint16_t* editedColor;
uint16_t emptyFlashCtr = 0;
uint16_t presetEditHoldMillis = 0;
bool presetEditorResetHold;
bool emptyFlashState = 0;
bool changingPreset;
bool mainCenterBtnReleased;
bool adjustingBrightness;
bool queuedTimeSave;
bool fireDebounce = 0;
bool updateCurrentPresetNextLoop;

#pragma endregion


//=======================UI========================
#pragma region UI Variables
inline void onSettings1Select(uint8_t);
UIList settings1 = UIList(&onSettings1Select,112,12,4,SettingsLabels,7);

inline void onEditPreset(uint8_t);
UIList presetList = UIList(&onEditPreset,66,12,4,PresetLabels,7);
inline void onAcceptPreset(int16_t value), onChangePresetVal(int16_t value);
UIInvisibleSlider presetSlider = UIInvisibleSlider(&onAcceptPreset,&onChangePresetVal,1,9999,50);

//UIList reloadModeList = UIList(&disp,[settings](uint8_t val){settings.reloadMode = val;openSettingsScreen();},102,12,4,&settings.uiColor,&settings.bgColor,ReloadModeLabels,5);

inline void onUISetupSelect(uint8_t);
UIList uiSetupList = UIList(&onUISetupSelect,112,12,4,UISetupLabels,7);

uint8_t selectedColorChannel;
uint16_t testColor;
inline void updateTestColor(int16_t);
inline void selectColorChannel(uint8_t channel);
inline void acceptColorChannel(int16_t);
UIInvisibleList colorEditorMenu = UIInvisibleList(&selectColorChannel,4);
//const uint16_t _const_red = ST7735_RED, _const_green = ST7735_GREEN, _const_blue = ST7735_BLUE;
UIColorSlider colorSliderR = UIColorSlider(ST7735_RED, 10,104,&acceptColorChannel,&updateTestColor,0,31,50);
UIColorSlider colorSliderG = UIColorSlider(ST7735_GREEN, 10,104,&acceptColorChannel,&updateTestColor,0,63,25);
UIColorSlider colorSliderB = UIColorSlider(ST7735_BLUE, 10,104,&acceptColorChannel,&updateTestColor,0,31,50);

inline void acceptTime();

inline void onSettings2Select(uint8_t);

UIList settings2 = UIList(&onSettings2Select,112,12,4,Settings2Labels,6);

UIInvisibleSlider timeSetupSlider = UIInvisibleSlider(&timeSetupSlider_Accept,&timeSetupSlider_OnValueChange,-127,127,200);
/*
UIInvisibleSlider calibrationSlider = UIInvisibleSlider(&acceptTime,&calibrationSlider_OnValueChange,-127,127,200);
UIInvisibleSlider minuteSlider = UIInvisibleSlider([calibrationSlider,settings,disp](){focusedUiElement = &calibrationSlider; calibrationSlider.value = settings.clockCalibration;disp.drawFastHLine(68,62,12,settings.bgColor);disp.drawFastHLine(52,88,24,settings.uiColor); },[disp, Rtc, minuteSlider](){Rtc.minutes = minuteSlider.value; disp.setCursor(50,50); printTime();},0,59,250);
UIInvisibleSlider hourSlider = UIInvisibleSlider([minuteSlider, Rtc, settings, disp](){focusedUiElement = &minuteSlider; minuteSlider.value = Rtc.minutes; disp.drawFastHLine(50,62,12,settings.bgColor);disp.drawFastHLine(68,62,12,settings.uiColor);},[disp, Rtc, settings, hourSlider](){Rtc.hours = hourSlider.value; disp.setCursor(50,50); printTime();},0,23,400);*/

UIList btList = UIList([settings](uint8_t val){settings.batteryType = val;openSettingsScreen();},102,12,4,BTLabels,6);

UIList abList = UIList([settings](uint8_t val){settings.selectedAmmoBar = val;openSettingsScreen2();},102,12,4,ABLabels,4);

#pragma endregion
//=================================================






void setup()
{
	
	loadSettings();
	analogReference(INTERNAL);
	
	pinMode(DSP_BL, OUTPUT);

	
	//Set MOSI and SCK as output (we will need to use them manually for display D/C transmission
	DDRB |= ((1 << PINB3) | (1 << PINB5));
	
	analogWrite(DSP_BL, 0); //Turn off display until initialization is comlete
	//disp.initR(INITR_144GREENTAB);
	disp.init();
	delay(10);
	//	disp.fillScreen(ST7735_RED);
	SPSR |= 1 << SPI2X;
	//SPCR = 0b11010000;
	SPCR &= ~0b00000011;
	disp.setRotation(settings.rotation);
	delay(50);
	disp.fillScreen(settings.bgColor);
	delay(50);
	setBrightness(brightness);
	
	//Setup pin change interrupt
	PCICR |= _BV(PCIE1);// | _BV(PCIE2); //Enable pc interrupt 1 (pins PCINT8-14)
	PCMSK1 |= _BV(PCINT8) | _BV(PCINT9) | _BV(PCINT10); //Mask PC interrupt pins so inly buttons are enabled
	//PCMSK2 |= _BV(PCINT17);
	
	ammo = settings.presets[currentPreset] < 0? 0 : settings.presets[currentPreset];


	PORTC |= (1 << PINC0) | (1 << PINC1) | (1 << PINC2); //pull up side buttons
	//PORTD |= _BV(RLD);
	

	attachInterrupt(SENSOR_INTERRUPT, &fire,CHANGE);
	PORTD |= _BV(PD2);
	attachInterrupt(BTN_RELOAD_INTERRUPT, &reloadInterrupt, CHANGE);
	Wire.begin();
	Rtc.Init();
	Rtc.Write(RTC_CALIBRATION,settings.clockCalibration);
	disp.fillScreen(settings.bgColor);
	disp.setTextColor(settings.uiColor, settings.bgColor);
	displayMainScreen();
	updateAmmoNextLoop = true;
	sei();
}

void loop()
{
	deltaMillis = millis() - currentMillis;
	currentMillis += deltaMillis;
	
	
	HANDLE_BUTTON_LOOP(BTN_LEFT);
	HANDLE_BUTTON_LOOP(BTN_RIGHT);
	HANDLE_BUTTON_LOOP(BTN_CENTER);
	HANDLE_BUTTON_LOOP(BTN_RLD)
	
	
	//Main screen loop
	if(currentScreen == SCREEN_MAIN)
	{
		//Ammo counter updates
		if(updateAmmoNextLoop)
		{
			if(updateAmmoNextLoop == 2)
			{
				disp.fillRect(0,ACN_STARTY, 128,52,settings.bgColor);
			}
			updateAmmoBar();
			
			uint8_t charset = 0;
			uint8_t _10, _100, _1000;
			uint16_t num = abs(ammo);
			uint8_t* d[5];
			disp.setCursor(0,0);

			if(settings.presets[currentPreset] > 999 || settings.presets[currentPreset] < 0){
				charset = 2;
				
				_1000 = num / 1000;
				num -= 1000* _1000;
				_100 = num / 100;
				num -= 100 * _100;
				_10 = num / 10;
				num -= 10 * _10;
				d[0] = C_Sdash;
				d[1] = C_Sdigits[_1000];
				d[2] = C_Sdigits[_100];
				d[3] = C_Sdigits[_10];
				d[4] = C_Sdigits[num];
			}
			else if (settings.presets[currentPreset] > 99){
				charset = 1;
				
				_100 = num / 100;
				num -= 100 * _100;
				_10 = num / 10;
				num -= 10 * _10;
				d[0] = C_Mdash;
				d[1] = C_Mdigits[_100];
				d[2] = C_Mdigits[_10];
				d[3] = C_Mdigits[num];
			}
			else
			{
				_10 = num / 10;
				num -= 10 * _10;
				d[0] = C_Ldash;
				d[1] = C_Ldigits[_10];
				d[2] = C_Ldigits[num];
			}
			uint8_t offset = ammo < 0 ? 0 : 1;

			disp.drawBitmap(digitPos[charset].x ,digitPos[charset].y,d[offset],ammoColor,settings.bgColor);
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx, digitPos[charset].y, d[2], ammoColor, settings.bgColor);
			if(!charset)goto FINISH_AMMO_UPDATE;
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx*2, digitPos[charset].y, d[3], ammoColor, settings.bgColor);
			if(charset == 1)goto FINISH_AMMO_UPDATE;
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx*3, digitPos[charset].y, d[4], ammoColor, settings.bgColor);
			
			FINISH_AMMO_UPDATE:
			//}
			updateAmmoNextLoop = 0;
		}
		
		if(!changingPreset)
		{
			//Brightness adjustment
			//NOTE: brightness is NOT SAVED AUTOMATICALLY DELIBERATELY to preserve the EEPROM memory, which has a limited number of possible erase/write cycles! The user can only save brightness manually to keep EEPROM load at minimum
			//****				  ************************************
			
			if(BTN_RIGHT_isPressed)
			{
				if(currentMillis > incrementMillis + INCREMENT_TIME)
				{
					incrementMillis = millis();
					setBrightness(brightness < 255? brightness+1 : 255);
					hideBrightnessMillis = millis();
					if(!adjustingBrightness){
						adjustingBrightness = 1;
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
					if(!adjustingBrightness){
						adjustingBrightness = 1;
						showBrightnessBar();
					}
					updateBrightnessBar();
				}
			}
			else if(adjustingBrightness && currentMillis > hideBrightnessMillis + BRIGHTNESS_STAY_TIME){
				hideBrightnessBar();
				adjustingBrightness = 0;
			}
		}
		if(updateCurrentPresetNextLoop)
		{
			updateCurrentPreset();
			updateCurrentPresetNextLoop = false;
		}
		
		//Empty Magazine Flash
		#ifdef EMPTY_MAGAZINE_FLASH_INTERVAL
		if(settings.emptyFlash &&  ammo <= 0 && settings.presets[currentPreset] > 0){
			emptyFlashCtr += deltaMillis;
			if(emptyFlashCtr >= EMPTY_MAGAZINE_FLASH_INTERVAL)
			{
				emptyFlashCtr = 0;
				emptyFlashState ^= 1;

				#ifdef EMF_STYLE_NUMBER_FLASH
				#ifdef EMF_SYNC_NUM_AND_BARS
				ammoColor = !emptyFlashState? settings.bgColor : settings.ctrColor3;
				#else
				ammoColor = emptyFlashState? settings.bgColor : settings.ctrColor3;
				#endif
				updateAmmoCount();
				#endif
				
				#ifdef EMF_STYLE_AMMO_BAR
				
				if(emptyFlashState)
				drawAmmoBar(128);
				else
				drawAmmoBar(0);
				
				#endif

			}
		}
		#endif // EMPTY_MAGAZINE_FLASH_INTERVAL
		
		//Battery measurement
		if(currentMillis > battMeasurementMillis + BATT_MEASURMENT_INTERVAL)
		{
			updateBattery();
			updateTime();
		}
		
		//Center button at main screen
		if(!BTN_CENTER_isPressed && !mainCenterBtnReleased)
		{
			mainCenterBtnReleased = 1;
			updateCurrentPreset();
			centerBtnMillis = millis();

		}
		if(currentMillis >  centerBtnMillis + CENTER_HOLD_TIME)
		{
			if(mainCenterBtnReleased)
			{
				changingPreset = 0;
				mainCenterBtnReleased = 0;
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
	}

	if(currentScreen == SCREEN_COLOR)
	if(selectedColorChannel != colorEditorMenu.selectedItem) updateColorEditorSelection();

	if(currentScreen == SCREEN_PRESETS){
		if(presetEditorResetHold)
		presetEditHoldMillis = 0;
		else
		presetEditorResetHold = true;
	}
}

//=======================Buttons========================
#pragma region Buttons
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
			changingPreset = 1;
			updateCurrentPresetNextLoop = true;
			centerBtnMillis = millis();
			if(settings.centerBtnReload)
			reload();
			break;
			
			case SCREEN_SYSINFO:
			openSettingsScreen();
			break;
		}
		return;
		
		case BTN_LEFT:
		if(currentScreen == SCREEN_MAIN && changingPreset){
			do
			{
				if(changedPreset)
				changedPreset--;
				else changedPreset = _PRESET_COUNT-1;
			} while (!settings.presets[changedPreset]);
			updateCurrentPresetNextLoop = true;
			centerBtnMillis = millis();
		}
		return;
		
		case BTN_RIGHT:
		if(currentScreen == SCREEN_MAIN && changingPreset){
			do
			{
				if(changedPreset < _PRESET_COUNT-1)
				changedPreset++;
				else changedPreset = 0;
			} while (!settings.presets[changedPreset]);
			updateCurrentPresetNextLoop = true;
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
	if(currentScreen == SCREEN_PRESETS)
	{
		presetEditorResetHold = false;
		presetEditHoldMillis = min(PRESET_EDIT_HOLD_TIME, presetEditHoldMillis +deltaMillis);
	}
	
	if(focusedUiElement){
		focusedUiElement->button_held(button,&buttonCtrs[button]);
		return;
	}
}
#pragma endregion
//======================================================

//====================Interrupts====================
#pragma region Interrupts
void reloadInterrupt()
{
	HANDLE_BUTTON_INTERRUPT(BTN_RLD,PINREGISTER_BTN_RELOAD,PIN_BTN_RELOAD);
}
ISR(PCINT1_vect)
{
	HANDLE_BUTTON_INTERRUPT(BTN_CENTER,PINREGISTER_BTN_CENTER,PIN_BTN_CENTER);
	HANDLE_BUTTON_INTERRUPT(BTN_LEFT,PINREGISTER_BTN_LEFT,PIN_BTN_LEFT);
	HANDLE_BUTTON_INTERRUPT(BTN_RIGHT,PINREGISTER_BTN_RIGHT,PIN_BTN_RIGHT);
}
ISR(PCINT2_vect)
{

}

void fire()
{
	cli();
	if(bit_is_clear(PINREGISTER_SENSOR, PIN_SENSOR))
	{
		if(!fireDebounce)
		{
			if (settings.presets[currentPreset] < 0)
			ammo++;
			else
			ammo--;
			
			//updateAmmo();
			updateAmmoNextLoop = 1;
			fireDebounce = 1;
		}
	}
	else
	{
		fireDebounce = 0;
	}
	sei();
}
void reload(){
	ammo = settings.presets[currentPreset] < 0? 0 : settings.presets[currentPreset];
	updateAmmoNextLoop = 2;
}
#pragma endregion
//==================================================

//====================Menu Selects====================
#pragma region Menu Selects


void onSettings1Select(uint8_t item) //Settings 1
{
	switch(item)
	{
		case 0:	//Save and exit
		saveSettings();
		displayMainScreen();
		return;
		
		case 1: //Manage presets
		openSimpleListScreen(&presetList,"Presets",SCREEN_PRESETS);
		//		disp.setTextColor(settings.uiColor,settings.bgColor);
		for (int i = 1; i < presetList.itemCount;i++)
		{
			disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*i);
			if (settings.presets[i-1] < 0) disp.print("UP");
			else if(settings.presets[i-1]) disp.print(settings.presets[i-1]);
			else disp.print("OFF");
		}
		return;
		
		case 2: //Set time
		openTimeSetup();
		return;
		
		case 3: //UI Setup
		openSimpleListScreen(&uiSetupList,"UI Settings",SCREEN_UISETUP);
		return;
		
		case 4:	//Battery selection
		btList.selectedItem = settings.batteryType;
		openSimpleListScreen(&btList,"Battery Types",SCREEN_BTSELECT,18,12);
		//		disp.setTextColor(settings.uiColor,settings.bgColor);
		disp.setCursor(8,btList.y+2+(btList.itemHeight+btList.spacing)*settings.batteryType);
		disp.print('>');
		
		//Reload behavior -disabled, replaced by battery selection
		/*reloadModeList.selectedItem = settings.reloadMode;
		openSimpleListScreen_xy(reloadModeList,"Reload Modes",SCREEN_RELOADMODES,18,12);
		disp.setTextColor(settings.uiColor,settings.bgColor);
		disp.setCursor(8,reloadModeList.y+2+(reloadModeList.itemHeight+reloadModeList.spacing)*settings.reloadMode);
		disp.print('>');*/
		return;
		
		case 5: //Save brightness
		eeprom_busy_wait();
		eeprom_update_byte(EA_BRIGHTNESS,brightness);
		disp.setTextColor(settings.bgColor,settings.uiColor); //CC
		disp.setCursor(105,2+settings1.y+(settings1.itemHeight+settings1.spacing)*5);
		disp.print("OK");
		disp.setTextColor(settings.uiColor,settings.bgColor); //RC
		return;
		
		case 6: //next page
		openSettingsScreen2();
		return;
	}
}

void onSettings2Select(uint8_t selection) //Settings 2
{
	switch (selection)
	{
		case 0:
		openSettingsScreen();
		break;
		
		
		case 1: //Select ammo bar style
		btList.selectedItem = settings.batteryType;
		openSimpleListScreen(&abList,"Ammo Bar Style",SCREEN_AMMOBARSELECT,18,12);
		disp.setCursor(8,abList.y+2+(abList.itemHeight+abList.spacing)*settings.selectedAmmoBar);
		disp.print('>');
		break;
		
		case 2: //Flip ammo bar direction
		settings.ammoBarDir ^= true;
		disp.setCursor(100,46);
		disp.setTextColor(settings.bgColor,settings.uiColor);
		disp.print(settings.ammoBarDir? ">>" : "<<");
		disp.setTextColor(settings.uiColor,settings.bgColor);
		break;
		
		case 3: //Toggle empty flash
		settings.emptyFlash ^= true;
		disp.setCursor(100,62);
		disp.setTextColor(settings.bgColor,settings.uiColor);
		disp.print(settings.emptyFlash? "ON " : "OFF");
		disp.setTextColor(settings.uiColor,settings.bgColor);
		break;
		
		case 4:
		settings.centerBtnReload = !settings.centerBtnReload;
		disp.setCursor(100,78);
		disp.setTextColor(settings.bgColor, settings.uiColor); //CC
		disp.print(settings.centerBtnReload? "ON " : "OFF");
		disp.setTextColor(settings.uiColor,settings.bgColor); //RC
		break;
		
		case 5: // Factory reset
		factoryResetCtr++;
		if(factoryResetCtr >= FACTORY_RESET_CONFIRMATIONS)
		{
			factoryReset();
			displayMainScreen();
		}
		else
		{
			disp.setCursor(100,94);
			disp.setTextColor(settings.bgColor, settings.uiColor); //CC
			disp.print('>');
			disp.print(FACTORY_RESET_CONFIRMATIONS - factoryResetCtr);
			disp.setTextColor(settings.uiColor,settings.bgColor); //RC
		}
		break;
	}
}

void onUISetupSelect(uint8_t item) //UI Setup
{
	switch(item)
	{
		case 0:
		openSettingsScreen();
		return;
		
		case 2:
		openColorEditor(&settings.uiColor);
		return;
		case 3:
		openColorEditor(&settings.bgColor);
		return;
		case 4:
		openColorEditor(&settings.ctrColor1);
		return;
		case 5:
		openColorEditor(&settings.ctrColor2);
		return;
		case 6:
		openColorEditor(&settings.ctrColor3);
		return;
		
		case 1:
		settings.rotation = settings.rotation >= 3 ? 0 : settings.rotation + 1;
		disp.setRotation(settings.rotation);
		openSimpleListScreen(&uiSetupList,"UI Settings",SCREEN_UISETUP);
	}
}

void acceptTime()
{
	queuedTimeSave = 1;
	openSettingsScreen();
}
void timeSetupSlider_OnValueChange()
{
	switch(timeSetupStage)
	{
		case 0:
		Rtc.hours = timeSetupSlider.value;
		disp.setCursor(50,50);
		printTime();
		break;
		
		case 1:
		Rtc.minutes = timeSetupSlider.value;
		disp.setCursor(50,50);
		printTime();
		break;
		
		case 2:
		settings.clockCalibration = timeSetupSlider.value;
		printCalibration();
		break;
	}
	//int8_t value = minuteSlider.value;
	//settings.clockCalibration = value;
	printCalibration();
}
void timeSetupSlider_Accept()
{
	switch(timeSetupStage)
	{
		case 0:
		timeSetupSlider.maxValue = 59;
		timeSetupSlider.value = Rtc.minutes;
		disp.drawFastHLine(50,62,12,settings.bgColor);
		disp.drawFastHLine(68,62,12,settings.uiColor);
		break;
		
		case 1:
		timeSetupSlider.maxValue = 127;
		timeSetupSlider.minValue = -127;
		timeSetupSlider.value = settings.clockCalibration;
		disp.drawFastHLine(68,62,12,settings.bgColor);
		disp.drawFastHLine(52,88,24,settings.uiColor);
		break;
		
		default:
		acceptTime();
		break;
	}
	timeSetupStage++;
}
#pragma endregion
//====================================================

//==============================Preset editor==============================
#pragma region Preset editor
void onEditPreset(uint8_t preset)
{
	if(!preset){
		openSettingsScreen();
		return;
	}
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(80,presetList.y+2+(presetList.itemHeight+presetList.spacing)*preset);
	disp.print('>');
	disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
	presetSlider.minValue = (preset-1) ? -1 : 1; //Set min value for preset #1 as 1 -> it musn't be turned off
	focusedUiElement = &presetSlider;
	presetSlider.value = settings.presets[presetList.selectedItem - 1];
	if(presetSlider.value < 0)disp.print("UP");
	else disp.print(presetSlider.value);
	disp.print("  ");
}
void onChangePresetVal(int16_t value)
{
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
	if(value < 0) disp.print("UP");
	else disp.print(value);
	disp.print(' '); //Print one space to remove any 3-character numbers

	if(value > 500 && presetEditHoldMillis >= PRESET_EDIT_HOLD_TIME) presetSlider.incrementInterval = INCREMENT_TIME_AMMO_500;
	else if(value > 100 && presetEditHoldMillis >= PRESET_EDIT_HOLD_TIME) presetSlider.incrementInterval = INCREMENT_TIME_AMMO_100;
	else presetSlider.incrementInterval = INCREMENT_TIME_AMMO;
}
void onAcceptPreset(int16_t value)
{
	settings.presets[presetList.selectedItem - 1] = value; //make sure the first preset is never off, otherwise the system will get stuck while searching for available presets
	focusedUiElement = &presetList;
	disp.writeFillRect(80,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem,10,10,settings.bgColor);
	if(!value){
		disp.setCursor(90,presetList.y+2+(presetList.itemHeight+presetList.spacing)*presetList.selectedItem);
		disp.print("OFF");
	}
}
#pragma endregion
//=========================================================================

//==============================Color editor==========================
#pragma region Color Editor
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
	//disp.setTextColor(settings.uiColor,settings.bgColor);
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
		disp.setTextColor(settings.uiColor, settings.bgColor);
		openSimpleListScreen(&uiSetupList,"UI Settings",SCREEN_UISETUP);
		return;
	}
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(43-((selectedColorChannel-1)*16),114);
	disp.print('^');
}
void updateColorEditorSelection()
{
	if(colorEditorMenu.selectedItem == 0)
	{
		disp.fillRect(64,100,56,12,settings.uiColor);
		disp.setTextColor(settings.bgColor,settings.uiColor); //CC
		disp.setCursor(80,102);
		disp.print("Back");
		disp.setTextColor(settings.uiColor,settings.bgColor); //RC
	}
	else
	{
		//disp.setTextColor(settings.uiColor,settings.bgColor);
		disp.setCursor(43-((colorEditorMenu.selectedItem-1)*16),114);
		disp.print('o');
	}
	
	if(selectedColorChannel != colorEditorMenu.selectedItem)
	{
		if(selectedColorChannel == 0)
		{
			disp.drawRect(64,100,56,12,settings.uiColor);
			disp.fillRect(65,101,54,10,settings.bgColor);
			//disp.setTextColor(settings.uiColor,settings.bgColor);
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
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.setCursor(43-((selectedColorChannel-1)*16),114);
	disp.print('o');
}
#pragma endregion
//====================================================================

//====================Display functions====================
#pragma region Display functions

void openSimpleListScreen(UIList* _list,char* title,uint8_t _screen,uint8_t _x,uint8_t _y){
	currentScreen = _screen;
	disp.fillScreen(settings.bgColor);
	//disp.setTextColor(settings.uiColor); //potential error
	disp.setCursor(40,0);
	disp.print(title);
	_list->draw(_x,_y);
	focusedUiElement = _list;
}

void openSettingsScreen()
{
	currentScreen = SCREEN_SETTINGS1;
	disp.fillScreen(settings.bgColor);
	//disp.setTextColor(settings.uiColor);//potential error
	disp.setCursor(40,0);
	disp.print("Settings");
	settings1.draw(8,12);
	focusedUiElement = &settings1;
}

void openSettingsScreen2()
{
	openSimpleListScreen(&settings2,"Settings 2",SCREEN_SETTINGS2);
	disp.setCursor(5,116);
	disp.print(F("Version: "));
	//disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&str));
	disp.print(VERSION);
}

void openTimeSetup()
{
	timeSetupStage = 0;
	focusedUiElement = &timeSetupSlider;
	timeSetupSlider.value = Rtc.hours;
	timeSetupSlider.maxValue = 23;
	timeSetupSlider.minValue = 0;
	disp.fillScreen(settings.bgColor);
	disp.setCursor(50,38);
	disp.print("HH:MM");
	disp.setCursor(50,50);
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	printTime();
	disp.drawFastHLine(50,62,12,settings.uiColor);
	disp.setCursor(30,66);
	disp.print("Calibration:");
	printCalibration();
}
void printTime()
{
	if(Rtc.hours < 10)disp.print('0');
	disp.print(Rtc.hours);
	disp.print(':');
	if(Rtc.minutes < 10)disp.print('0');
	disp.print(Rtc.minutes);

}
void printCalibration()
{
	disp.setCursor(52,78);
	disp.print(settings.clockCalibration);
	disp.print(' ');
}
#pragma endregion
//=========================================================

//====================Main Screen Functions====================
#pragma region Main Screen Functions
void displayMainScreen()
{
	focusedUiElement = nullptr;
	currentScreen = SCREEN_MAIN;
	disp.fillScreen(settings.bgColor);
	//disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.drawBitmap(88,112,IconBattery,settings.uiColor,settings.bgColor);
	disp.drawBitmap(37,112,IconClock,settings.uiColor,settings.bgColor);
	updateAmmoBar();
	updateAmmoNextLoop = true;
	updateCurrentPreset();
	factoryResetCtr = 0;
}

void updateAmmoBar()
{
	uint8_t size;
	if(settings.presets[currentPreset] < 0)
	{
		size = 0;
		ammoColor = settings.ctrColor1;
	}
	else
	{
		size = ammo > 0 ? map(ammo,0,settings.presets[currentPreset],0,128) : 0;
		
		if(size > 85) ammoColor = settings.ctrColor1;
		else if(size > 42) ammoColor = settings.ctrColor2;
		else ammoColor = settings.ctrColor3;
	}
	drawAmmoBar(size);
}

void drawAmmoBar(uint8_t size)
{
	if(size == 0){
		disp.fillRect(0,0,128,20,settings.bgColor);
		return;
	}
	if(size == 128){
		disp.drawBitmap(0,0,AmmoBars[settings.selectedAmmoBar],ammoColor,settings.bgColor);
		return;
	}
	int16_t crop = 128-size;
	disp.drawBitmap(0,0,AmmoBars[settings.selectedAmmoBar],ammoColor,settings.bgColor,settings.ammoBarDir? -crop : crop,0);
}
void setBrightness(uint8_t newBrightness)
{
	brightness = newBrightness;
	analogWrite(DSP_BL, brightness);
}

void showBrightnessBar()
{
	disp.drawBitmap(2,28,IconBrightness,settings.uiColor,settings.bgColor);
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

void updateTime()
{
	disp.setCursor(6,112);
	if(queuedTimeSave)
	{
		disp.print("FAIL");
		disp.setCursor(6,112);
		Rtc.SetTime();
		queuedTimeSave = 0;
	}else
	Rtc.Update();
	printTime();
	battMeasurementMillis = millis();
}

void updateBattery()
{

	uint16_t result = analogRead(BATTERY_ADC_CHANNEL);
	float voltage = (result * 11 / 1024.0f) *1.127f;
	
	disp.setCursor(102,112);
	//	disp.setFont();
	//disp.setTextSize(1);
	//disp.setTextColor(settings.uiColor,settings.bgColor);

	if(settings.batteryType)
	{
		uint8_t percentage = 255;
		uint8_t cells = 1;
		switch(settings.batteryType)
		{
			case BT_NIMH_7CELL:
			cells = 7;
			break;
			
			case BT_NIMH_8CELL:
			cells = 8;
			break;
			
			case BT_LIPO_7V4:
			cells = 2;
			break;
			
			case BT_LIPO_11V1:
			cells = 3;
			break;
		}
		switch(settings.batteryType)
		{
			case BT_NIMH_7CELL:
			case BT_NIMH_8CELL:
			percentage = calculateBatteryPrecentage(voltage/cells,NIMH_H_OFFSET,NIMH_K, NIMH_BASE, NIMH_V_OFFSET);
			break;
			
			case BT_LIPO_7V4:
			case BT_LIPO_11V1:
			percentage = calculateBatteryPrecentage(voltage/cells,LIPO_H_OFFSET,LIPO_K, LIPO_BASE, LIPO_V_OFFSET);
			break;
			
			case BT_StA_9VOLT:
			percentage = max(0, min(100, 100.0f * (voltage - StA_9V_MIN) / (StA_9V_MAX - StA_9V_MIN)));
			break;
			
		}
		if(percentage < 15) disp.setTextColor(settings.ctrColor3, settings.bgColor); //change color = CC
		disp.print(percentage,DEC);
		disp.print('%');
		if(percentage < 15) disp.setTextColor(settings.uiColor, settings.bgColor); //revert change = RC
		disp.fillRect(disp.getCursorX(),disp.getCursorY(),5,7,settings.bgColor);
	}
	else
	disp.print(voltage,1); //Raw voltage
}

void updateCurrentPreset()
{
	//disp.setTextColor(settings.uiColor, settings.bgColor);
	disp.setCursor(59,109);
	disp.setTextSize(2);
	disp.print(changingPreset? changedPreset+1 : currentPreset+1);
	disp.setTextSize(1);
	
	disp.setTextColor(changingPreset? settings.uiColor : settings.bgColor); //CC
	//uint8_t orgCursorX =  disp.getCursorX();
	//uint8_t orgCursorY =  disp.getCursorY();
	disp.setCursor(49,109);
	disp.print('<');
	disp.setCursor(73,109);
	disp.print('>');
	//disp.setCursor(orgCursorX,orgCursorY);
	disp.setTextColor(settings.uiColor,settings.bgColor); //RC
}
#pragma endregion
//=============================================================

//====================Math====================
#pragma region Math Functions
uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset)
{
	float percent = (1/(pow(base,h_offset - voltage/k)+1)) + v_offset;
	return min(100,(uint8_t)(100*percent));
}
#pragma endregion
//============================================

//====================System functions====================
#pragma region System functions

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
	
	__TachyonSettings factorySettings;
	factorySettings.uiColor = FACT_UICOLOR;
	factorySettings.bgColor = ST7735_BLACK;
	factorySettings.ctrColor1 = FACT_CTRCOLOR1;
	factorySettings.ctrColor2 = FACT_CTRCOLOR2;
	factorySettings.ctrColor3 = FACT_CTRCOLOR3;
	
	factorySettings.presets[0] = FACT_PRESET1;
	factorySettings.presets[1] = FACT_PRESET2;
	factorySettings.presets[2] = FACT_PRESET3;
	factorySettings.presets[3] = FACT_PRESET4;
	factorySettings.presets[4] = FACT_PRESET5;
	factorySettings.presets[5] = FACT_PRESET6;

	factorySettings.rotation = FACT_UIROTATION;
	factorySettings.selectedAmmoBar = FACT_AMMOBAR;
	factorySettings.ammoBarDir = FACT_AMMOBARDIR;
	factorySettings.emptyFlash = FACT_EMFLASH;
	factorySettings.batteryType = FACT_BATTERY;
	factorySettings.centerBtnReload = FACT_CENTERBTNRELOAD;
	factorySettings.clockCalibration = 0;
	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		eeprom_write_byte(EA_SETTINGS+i, ((uint8_t*)&factorySettings)[i]);
	}
	loadSettings();
}

/*
void soft_reset()
{
cli();
asm("ijmp" ::"z" (0x0000));//(0x3F00));
}*/
#pragma endregion
//========================================================