/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

*/
#include "FirmwareCore/TachyonFirmware.h"
#include "FirmwareCore/Definitions.h"

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
	

	attachInterrupt(1,&fire,CHANGE);
	PORTD |= _BV(PD2);
	attachInterrupt(0, &reloadInterrupt, CHANGE);
	Wire.begin();
	Rtc.Init();
	Serial.begin(9600);
	disp.fillScreen(settings.bgColor);
	displayMainScreen();
	updateAmmoCount();
	sei();
}

void loop()
{
	deltaMillis = millis() - currentMillis;
	currentMillis += deltaMillis;
	
	if(Serial.available())
	executeCommand(Serial.read());
	
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
				disp.fillRect(1,49,126,48,settings.bgColor);
			}
			updateAmmoBar();
			/*if(settings.largeNumber){
			//16, 56
			disp.setTextColor(ammoColor, settings.bgColor);
			disp.setCursor(16,56);
			disp.setTextSize(4);
			disp.print(ammo);
			disp.setTextSize(1);
			disp.setTextColor(settings.uiColor, settings.bgColor);
			}
			else
			{*/
				if(settings.presets[currentPreset] < 100 && settings.presets[currentPreset] > 0)
				{
					uint8_t _10 = abs(ammo) / 10;
					uint8_t _1 = abs(ammo) - (10 * _10);
					
					if(_1 == 1)
					disp.drawFastBitmap(66,49,C_L1R,ammoColor,settings.bgColor);
					else if (magOut)
					disp.drawFastBitmap(66,49,C_Ldash,ammoColor,settings.bgColor);
					else
					disp.drawFastBitmap(66,49,C_Lnums[_1],ammoColor,settings.bgColor);
					
					if(ammo < 0 || magOut)
					disp.drawFastBitmap(14,49,C_Ldash,ammoColor,settings.bgColor);
					else
					disp.drawFastBitmap(14,49,C_Lnums[_10],ammoColor,settings.bgColor);
					

				}
				else if(settings.presets[currentPreset] < 1000)
				{
					if(ammo > 999) ammo = 999;
					uint8_t _100 = abs(ammo) / 100;
					uint8_t _10 = (abs(ammo) - _100 * 100) / 10;
					uint8_t _1 = abs(ammo) - (10 * _10) - (_100 * 100);
					
					if(_1 == 1)
					disp.drawFastBitmap(89,54,C_S1R,ammoColor,settings.bgColor);
					else if (magOut)
					disp.drawFastBitmap(89,54,C_Sdash,ammoColor,settings.bgColor);
					else
					disp.drawFastBitmap(89,54,C_Snums[_1],ammoColor,settings.bgColor);
					
					if(magOut)
					disp.drawFastBitmap(45,54,C_Sdash,ammoColor,settings.bgColor);
					else
					disp.drawFastBitmap(45,54,C_Snums[_10],ammoColor,settings.bgColor);
					
					if(_100 == 1)
					disp.drawFastBitmap(1,54,C_S1L,ammoColor,settings.bgColor);
					else if (magOut || ammo < 0)
					disp.drawFastBitmap(1,54,C_Sdash,ammoColor,settings.bgColor);
					else
					disp.drawFastBitmap(1,54,C_Snums[_100],ammoColor,settings.bgColor);
				}
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
			
			//Empty Magazine Flash
			#ifdef EMPTY_MAGAZINE_FLASH_INTERVAL
			if(ammo <= 0 && settings.presets[currentPreset] > 0){
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
					#ifdef EMF_STYLE_HAZARDLIGHTS
					if(emptyFlashState){
						disp.drawFastBitmap(64,112,AmmoBar1R, settings.ctrColor3, settings.bgColor);
						disp.fillRect(0,112,64,16,settings.bgColor);
					}
					else
					{
						disp.drawFastBitmap(0,112,AmmoBar1L, settings.ctrColor3, settings.bgColor);
						disp.fillRect(64,112,64,16,settings.bgColor);
					}
					#elif defined(EMF_STYLE_BOTTOM_BAR)
					if(emptyFlashState){
						disp.drawFastBitmap(64,112,AmmoBar1R, settings.ctrColor3, settings.bgColor);
						disp.drawFastBitmap(0,112,AmmoBar1L, settings.ctrColor3, settings.bgColor);
					}
					else
					{
						disp.fillRect(0,112,128,16,settings.bgColor);
					}
					#endif
				}
			}
			#endif // EMPTY_MAGAZINE_FLASH_INTERVAL
			
			//Battery measurement
			if(currentMillis > battMeasurementMillis + BATT_MEASURMENT_INTERVAL){
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
				battMeasurementMillis = millis();
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
				updateCurrentPreset();
				centerBtnMillis = millis();
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
				updateCurrentPreset();
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

	void fire()
	{
		cli();
		if(bit_is_clear(PIND,PIND3))
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
	void updateAmmoCount()
	{
		updateAmmoNextLoop = true;
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
			openSimpleListScreen(presetList,"Presets",SCREEN_PRESETS);
			disp.setTextColor(settings.uiColor,settings.bgColor);
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
			openSimpleListScreen(uiSetupList,"UI Settings",SCREEN_UISETUP);
			return;
			
			case 4:	//Battery selection
			btList.selectedItem = settings.batteryType;
			openSimpleListScreen_xy(btList,"Battery Types",SCREEN_BTSELECT,18,12);
			disp.setTextColor(settings.uiColor,settings.bgColor);
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
			disp.setTextColor(settings.bgColor,settings.uiColor);
			disp.setCursor(105,2+settings1.y+(settings1.itemHeight+settings1.spacing)*5);
			disp.print("OK");
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
			
			/*case 1:
			settings.countUp ^= 1;
			disp.setCursor(10,30);
			disp.setTextColor(settings.bgColor, settings.uiColor);
			disp.print(">Current dir:");
			disp.print(settings.countUp? "Up" : "Dn");
			disp.setTextColor(settings.uiColor,settings.bgColor);
			break;*/
			
			case 1: // Factory reset
			factoryResetCtr++;
			if(factoryResetCtr >= FACTORY_RESET_CONFIRMATIONS)
			{
				factoryReset();
				displayMainScreen();
			}
			else
			{
				disp.setCursor(100,30);
				disp.setTextColor(settings.bgColor, settings.uiColor);
				disp.print('>');
				disp.print(FACTORY_RESET_CONFIRMATIONS - factoryResetCtr);
				disp.setTextColor(settings.uiColor,settings.bgColor);
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
			openSimpleListScreen(uiSetupList,"UI Settings",SCREEN_UISETUP);
		}
	}

	void acceptTime()
	{
		queuedTimeSave = 1;
		openSettingsScreen();
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
		disp.setTextColor(settings.uiColor,settings.bgColor);
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
		disp.setTextColor(settings.uiColor,settings.bgColor);
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
	#pragma endregion
	//====================================================================

	//====================Display functions====================
	#pragma region Display functions

	void openSettingsScreen()
	{
		currentScreen = SCREEN_SETTINGS1;
		disp.fillScreen(settings.bgColor);
		disp.setTextColor(settings.uiColor);
		disp.setCursor(40,0);
		disp.print("Settings");
		settings1.draw(8,12);
		focusedUiElement = &settings1;
	}

	void openSettingsScreen2()
	{
		openSimpleListScreen(settings2,F("Settings 2"),SCREEN_SETTINGS2);
		disp.setCursor(5,116);
		disp.print(F("Version: "));
		
		//disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&str));
		disp.print(VERSION);
	}

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

	#pragma endregion
	//=========================================================

	//====================Main Screen Functions====================
	#pragma region Main Screen Functions
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
		factoryResetCtr = 0;
	}
	void updateAmmoBar()
	{
		uint8_t crop;
		if(settings.presets[currentPreset] < 0)
		{
			crop = 64;
			ammoColor = settings.ctrColor1;
		}
		else
		{
			crop = ammo > 0 ? 64-map(ammo,0,settings.presets[currentPreset],0,64) : 64;
			
			if(crop > 42) ammoColor = settings.ctrColor3;
			else if(crop > 21) ammoColor = settings.ctrColor2;
			else ammoColor = settings.ctrColor1;
		}
		
		disp.drawFastBitmapCropped(0,112,AmmoBar1L,ammoColor,settings.bgColor,crop,0);
		disp.drawFastBitmapCropped(64,112,AmmoBar1R,ammoColor,settings.bgColor,-crop,0);
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
		//disp.setTextSize(1);
		disp.setTextColor(settings.uiColor,settings.bgColor);

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
			if(percentage < 15) disp.setTextColor(settings.ctrColor3, settings.bgColor);
			disp.print(percentage,DEC);
			disp.print('%');
			disp.setTextColor(settings.uiColor, settings.bgColor);
			disp.fillRect(disp.getCursorX(),disp.getCursorY(),5,7,settings.bgColor);
		}
		else
		disp.print(voltage,1); //Raw voltage
	}
	void updateCurrentPreset()
	{
		disp.setTextColor(settings.uiColor, settings.bgColor);
		disp.setCursor(105,13);
		disp.setTextSize(2);
		disp.print(changingPreset? changedPreset+1 : currentPreset+1);
		disp.setTextSize(1);
		
		disp.setTextColor(changingPreset? settings.uiColor : settings.bgColor);
		disp.setCursor(95,22);
		disp.print('<');
		disp.setCursor(119,22);
		disp.print('>');
		
		disp.setTextColor(settings.uiColor,settings.bgColor);
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

	//====================USB Interface====================
	#pragma region USB Interface
	void executeCommand(uint8_t cmd)
	{
		/*switch(cmd)
		{
		default:
		disp.print("UNC");
		break;
		
		case 0:
		disp.print("GVR");
		Serial.println(VERSION);
		break;
		
		case 1: //GET_TIME
		disp.print("GTM");
		Serial.write(Rtc.seconds);
		Serial.write(Rtc.minutes);
		Serial.write(Rtc.hours);
		break;

		case 5: //SET_RTC_CALIBRATION
		disp.print("STC");
		disp.setCursor(20,30);
		disp.print("Read: ");
		int8_t calibration = Serial.read();
		Rtc.Write(RTC_CALIBRATION, calibration);
		disp.print(calibration, HEX);
		disp.print(" SET!");
		break;

		case 4: //GET_RTC_CALIBRATION
		disp.print("GTC");
		int8_t currentcalibration = Rtc.Read(RTC_CALIBRATION);
		disp.setCursor(20,40);
		disp.print(currentcalibration, HEX);
		Serial.write(currentcalibration);
		break;
		
		}*/
		
		//For some reason the switch above caused problems, where cases below 0x05 were always ignored during compilation (why??!) so instead I used this painful if-else contraption
		if(cmd == 0x00){ //GVR
			Serial.println(VERSION);
		}
		else if (cmd == 0x01){ //GTM
			Serial.write(Rtc.seconds);
			Serial.write(Rtc.minutes);
			Serial.write(Rtc.hours);
		}
		else if (cmd == 0x04){ //GTC
			int8_t currentcalibration = Rtc.Read(RTC_CALIBRATION);
			Serial.write(currentcalibration);
		}
		else if (cmd == 0x05){ //STC
			/*	disp.setCursor(20,30);
			disp.print("STC: ");*/
			while(!Serial.available()){} //wait until transmission finishes
			int8_t calibration = Serial.read();
			Rtc.Write(RTC_CALIBRATION, calibration);
			/*disp.print(calibration, HEX);
			disp.print(" SET!");*/
		}
	}
	#pragma endregion
	//=====================================================

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

	/*
	void soft_reset()
	{
	cli();
	asm("ijmp" ::"z" (0x0000));//(0x3F00));
	}*/
	#pragma endregion
	//========================================================