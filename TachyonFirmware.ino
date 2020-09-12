/*
OFFICIAL TACHYON FIRMWARE
Version 1.x

(c) Martin Hrehor 2018

*/
#include "FirmwareCore/Definitions.h"
#include "FirmwareCore/SampleBuffer.h"
#include "FirmwareCore/TachyonFirmware.h"

__TachyonSettings settings;

const uint16_t CharsetLengths[] = {290, 183, 108};
const uint8_t *Charsets[] = {(uint8_t *)C_Ldigits, (uint8_t *)C_Mdigits, (uint8_t *)C_Sdigits};
const uint8_t *Dashes[] = {(uint8_t *)C_Ldash, (uint8_t *)C_Mdash, (uint8_t *)C_Sdash};

#pragma region Global Variables

// Variables
uint8_t factoryResetCtr;
uint8_t updateAmmoNextLoop;
uint8_t currentScreen;
uint8_t selectedListItem = 0;
Adafruit_ST7735 disp = Adafruit_ST7735(DSP_CS, 11, DSP_RST);
unsigned long currentMillis, incrementMillis, hideBrightnessMillis, centerBtnMillis, battMeasurementMillis;
unsigned long deltaMillis;
unsigned long delayMeterMicros;
long delayMicros;
RTC Rtc = RTC();
uint8_t currentPreset, changedPreset;
int16_t ammo;
uint16_t ammoColor;
uint8_t brightness;
int8_t batteryPercentage = 127;
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
bool isMenuListOpen;
bool lastHAssistColor = false;
SampleBuffer<uint16_t> batteryBuffer = SampleBuffer<uint16_t>(10);

#ifdef HAS_MAGNETOMETER
LSM303AGR mag = LSM303AGR();
Vector3f mc_min;
Vector3f mc_max;
#endif

#pragma endregion

//=======================UI========================
#pragma region UI Variables

const char *const *const ScreenToMenuContents[] = {
	nullptr,			//  SCREEN_MAIN 0
	SettingsLabels,		//  SCREEN_SETTINGS1 1
	PresetLabels,		//  SCREEN_PRESETS 2
	ColorEditorLabels,	//  SCREEN_COLOR 3
	UISetupLabels,		//  SCREEN_UISETUP 4
	nullptr,			//  SCREEN_TIME 5
	Settings2Labels,	//  SCREEN_SETTINGS2 6
	BTLabels,			//  SCREEN_BTSELECT 7
	ABLabels,			//  SCREEN_AMMOBARSELECT 8
	nullptr,			//  SCREEN_EDITVALUE 9
	nullptr,			//	SCREEN_MAG_CONF 10
	nullptr,			//	SCREEN_DELAYMETER 11
	Settings3Labels,	//  SCREEN_SETTINGS3 12
	CMSettings			//  SCREEN_COMPASS_SETTINGS 13
};
// This array holds the item count for all the menus that can be opened to be
// used at runtime (since their size is known only in compile-time)
const size_t ScreenToMenuItemCount[] = {
	0,											//  SCREEN_MAIN 0
	sizeof(SettingsLabels) / sizeof(size_t),	//  SCREEN_SETTINGS1 1
	sizeof(PresetLabels) / sizeof(size_t),		//  SCREEN_PRESETS 2
	sizeof(ColorEditorLabels) / sizeof(size_t), //  SCREEN_COLOR 3
	sizeof(UISetupLabels) / sizeof(size_t),		//  SCREEN_UISETUP 4
	0,											//  SCREEN_TIME 5
	sizeof(Settings2Labels) / sizeof(size_t),   //  SCREEN_SETTINGS2 6
	sizeof(BTLabels) / sizeof(size_t),			//  SCREEN_BTSELECT 7
	sizeof(ABLabels) / sizeof(size_t),			//  SCREEN_AMMOBARSELECT 8
	0,											//  SCREEN_EDITVALUE 9
	0,											//	SCREEN_MAG_CONF 10
	0,											//	SCREEN_DELAYMETER 11
	sizeof(Settings3Labels) / sizeof(size_t),	//  SCREEN_SETTINGS3 12
	sizeof(CMSettings) / sizeof(size_t)			//  SCREEN_COMPASS_SETTINGS 13
};

// Used for pick-selection menus
uint8_t *selectionTarget;

// Value editor
int *editedValue;
int editedValueMax, editedValueMin;
uint8_t editedValueIncrement;
uint8_t previousScreen;
uint8_t editedValueCtr;

// Color editing
int16_t editedColorComponents[3];
uint16_t *editedColor;

//Magnetometer calibration
uint8_t lastMCindicatorX[3] = {128 / 2, 128 / 2, 128 / 2};

#pragma endregion
//=================================================

void setup()
{
	loadSettings();
	analogReference(INTERNAL);

	pinMode(DSP_BL, OUTPUT);

	// Set MOSI and SCK as output (we will need to use them manually for display
	// D/C transmission
	DDRB |= ((1 << PINB3) | (1 << PINB5));

	analogWrite(DSP_BL, 0); // Turn off display until initialization is complete
	// disp.initR(INITR_144GREENTAB);
	disp.init();
	delay(10);
	//	disp.fillScreen(ST7735_RED);
	SPSR |= 1 << SPI2X;
	// SPCR = 0b11010000;
	SPCR &= ~0b00000011;
	disp.setRotation(settings.rotation);
	delay(50);
	disp.fillScreen(settings.bgColor);
	delay(50);
	setBrightness(brightness);

	// TODO: Add compile time branch for V3
	// Setup pin change interrupt
	PCICR |= _BV(PCIE1) |
	_BV(PCIE3); // | _BV(PCIE2); //Enable pc interrupt 1 (pins PCINT8-14)
	PCMSK1 |= _BV(PCINT8) | _BV(PCINT9) |
	_BV(PCINT10); // Mask PC interrupt pins so inly buttons are enabled
	PCMSK3 = _BV(PCINT27);
	// PCMSK2 |= _BV(PCINT17);

	ammo = settings.presets[currentPreset] < 0 ? 0 : settings.presets[currentPreset];

	// TODO: Add compile time branch for V3
	PORTE |= 1 << PINE3;
	PORTC |= (1 << PINC0) | (1 << PINC1); // pull up side buttons
	// PORTD |= _BV(RLD);

	SENSOR_PORT |= _BV(SENSOR_PIN);
	attachInterrupt(SENSOR_INTERRUPT, &fire, CHANGE);
	RELOAD_PORT |= _BV(RELOAD_PIN);
	attachInterrupt(RELOAD_INTERRUPT, &reloadInterrupt, CHANGE);
	Wire.begin();
	Rtc.Init();
	Rtc.Write(RTC_CALIBRATION, settings.clockCalibration);
	disp.fillScreen(settings.bgColor);
	disp.setTextColor(settings.uiColor, settings.bgColor);
	displayMainScreen();
	updateAmmoNextLoop = true;

	#ifdef HAS_MAGNETOMETER
	mag.init();
	mag.offset = settings.magOffset;
	#endif

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

	// Main screen loop
	switch (currentScreen)
	{
		case SCREEN_MAIN:
		// Ammo counter updates
		if (updateAmmoNextLoop)
		{
			if (updateAmmoNextLoop == 2)
			{
				disp.fillRect(0, ACN_STARTY, 128, 52, settings.bgColor);
			}
			updateAmmoBar();
			//#define USE_OLD_DECOMP_METHOD
			#ifdef USE_OLD_DECOMP_METHOD

			uint8_t charset = 0;
			uint8_t _10, _100, _1000;
			uint16_t num = abs(ammo);
			uint8_t *d[5];

			if (settings.presets[currentPreset] > 999 || settings.presets[currentPreset] < 0)
			{
				charset = 2;

				_1000 = num / 1000;
				num -= 1000 * _1000;
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
			else if (settings.presets[currentPreset] > 99)
			{
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

			disp.drawBitmap(digitPos[charset].x, digitPos[charset].y, d[offset], ammoColor, settings.bgColor);
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx, digitPos[charset].y, d[2], ammoColor, settings.bgColor);
			if (!charset)
			goto FINISH_AMMO_UPDATE;
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx * 2, digitPos[charset].y, d[3], ammoColor, settings.bgColor);
			if (charset == 1)
			goto FINISH_AMMO_UPDATE;
			disp.drawBitmap(digitPos[charset].x + digitPos[charset].spacingx * 3, digitPos[charset].y, d[4], ammoColor, settings.bgColor);

			FINISH_AMMO_UPDATE:
			#else

			//Digit decomposition:
			uint8_t digits[] = {0, 0, 0, 0}; //<< 1000-s, 100-s, 10-s, 1-s

			uint16_t num = abs(ammo);			   // << 2;		   //Shift by 2, since max ammo is 9999 -> the most significant bit is 14th from right. This will save 2 iterations in the following division
			uint16_t divisors[] = {1000, 100, 10}; //Array to pick divisors from memory without using logic.
			uint8_t digitCount = 2 + (settings.presets[currentPreset] >= 100) + (settings.presets[currentPreset] >= 1000) + (settings.presets[currentPreset] == -1)*2;
			for (char d = 4 /*- max number of digits*/ - digitCount; d < 3 /*- number of elements in divisors[]*/; d++)
			{
				//Binary divison:
				uint16_t r = 0;
				for (char i = 15; i >= 0; i--)
				{
					r = r << 1;
					r |= (num >> i) & 1;
					if (r >= divisors[d])
					{
						r -= divisors[d];
						digits[d] |= 1 << i;
					}
				}
				num = r;
			}
			digits[3] = num; //set last digit to the division remainder, since we won't be dividing by 1 in the previous step ...
			for (char i = 0; i < digitCount; i++)
			{
				/*	uint8_t *digit, *dash;
				switch (digitCount)
				{
				case 2:
				digit = C_Ldigits[digits[3 - i]];
				dash = C_Ldash;
				break;
				case 3:
				digit = C_Mdigits[digits[3 - i]];
				dash = C_Mdash;
				break;
				default:
				digit = C_Sdigits[digits[3 - i]];
				dash = C_Sdash;
				break;
				}*/
				uint8_t *digit = (i == digitCount - 1) && ammo < 0 ? Dashes[digitCount - 2] : Charsets[digitCount - 2] + CharsetLengths[digitCount - 2] * digits[3 - i];
				__digitPositioning pos = digitPos[digitCount - 2];
				disp.drawBitmap(pos.x + pos.spacingx * (digitCount - 1 - i), pos.y, /*((i == digitCount - 1) && ammo < 0) ? dash : */ digit, ammoColor, settings.bgColor);
			}
			#endif
			updateAmmoNextLoop = 0;
		}

		if (!changingPreset)
		{
			// Brightness adjustment
			// NOTE: brightness is NOT SAVED AUTOMATICALLY DELIBERATELY to
			// preserve the EEPROM memory, which has a limited number of possible
			// erase/write cycles! The user can only save brightness manually to
			// keep EEPROM load at minimum
			//****
			//************************************

			/*if (BTN_RIGHT_isPressed)
			{
			if (currentMillis > incrementMillis + BRIGHTNESS_INCREMENT_INTERVAL)
			{
			incrementMillis = millis();
			setBrightness(brightness < 255 ? brightness + 1 : 255);
			hideBrightnessMillis = millis();
			if (!adjustingBrightness)
			{
			adjustingBrightness = 1;
			showBrightnessBar();
			}
			updateBrightnessBar();
			}
			}
			else if (BTN_LEFT_isPressed)
			{
			if (currentMillis > incrementMillis + BRIGHTNESS_INCREMENT_INTERVAL)
			{
			incrementMillis = millis();
			setBrightness(brightness > MIN_BRIGHTNESS ? brightness - deltaMillis /BRIGHTNESS_INCREMENT_INTERVAL : MIN_BRIGHTNESS);
			hideBrightnessMillis = millis();
			if (!adjustingBrightness)
			{
			adjustingBrightness = 1;
			showBrightnessBar();
			}
			updateBrightnessBar();
			}
			}
			else if (adjustingBrightness && currentMillis > hideBrightnessMillis + BRIGHTNESS_STAY_TIME)
			{
			hideBrightnessBar();
			adjustingBrightness = 0;
			}*/
		}
		if (updateCurrentPresetNextLoop)
		{
			updateCurrentPreset();
			updateCurrentPresetNextLoop = false;
		}

		// Empty Magazine Flash
		#ifdef EMPTY_MAGAZINE_FLASH_INTERVAL
		if (settings.emptyFlash && ammo <= 0 && settings.presets[currentPreset] > 0)
		{
			emptyFlashCtr += deltaMillis;
			if (emptyFlashCtr >= EMPTY_MAGAZINE_FLASH_INTERVAL)
			{
				emptyFlashCtr = 0;
				emptyFlashState ^= 1;

				#ifdef EMF_STYLE_NUMBER_FLASH
				#ifdef EMF_SYNC_NUM_AND_BARS
				ammoColor =
				!emptyFlashState ? settings.bgColor : settings.ctrColor3;
				#else
				ammoColor =
				emptyFlashState ? settings.bgColor : settings.ctrColor3;
				#endif
				updateAmmoCount();
				#endif

				#ifdef EMF_STYLE_AMMO_BAR

				if (emptyFlashState)
				drawAmmoBar(128);
				else
				drawAmmoBar(0);

				#endif
			}
		}
		#endif // EMPTY_MAGAZINE_FLASH_INTERVAL

		// Battery measurement
		if (currentMillis > battMeasurementMillis + BATT_MEASURMENT_INTERVAL)
		{
			updateBattery();
			updateTime();
			if(settings.compassMode)
			updateCompass();
			if(settings.hAssistEnabled)
			updateHAssist();
		}

		// Center button at main screen
		if (!BTN_CENTER_isPressed && !mainCenterBtnReleased)
		{
			mainCenterBtnReleased = 1;
			updateCurrentPreset();
			centerBtnMillis = millis();
		}
		if (currentMillis > centerBtnMillis + CENTER_HOLD_TIME)
		{
			if (mainCenterBtnReleased)
			{
				changingPreset = 0;
				mainCenterBtnReleased = 0;
				if (changedPreset != currentPreset)
				{
					currentPreset = changedPreset;
					reload();
				}
			}
			else if (BTN_CENTER_isPressed)
			{
				openMenuListScreen(SCREEN_SETTINGS1);
				// openSimpleListScreen(list1,"Settings",SCREEN_SETTINGS);
			}
		}
		break;

		case SCREEN_DELAYMETER:
		if(updateAmmoNextLoop)
		{
			updateAmmoNextLoop = false;
			if(delayMicros > 0)
			{
				disp.fillRect(64,20,64,24, settings.bgColor);
				disp.setCursor(64,20);
				disp.print(delayMicros / 1000.0F, 3); //delay [ms]
				disp.setCursor(64,36);
				disp.print(60000000.0F / (delayMicros)); //Fire rate [RPM]
			}
		}
		
		case SCREEN_PRESETS:

		if (presetEditorResetHold)
		presetEditHoldMillis = 0;
		else
		presetEditorResetHold = true;
		break;
		
		#ifdef HAS_MAGNETOMETER
		case SCREEN_MAG_CAL:
		mag.read3Axis(MAG);
		mc_min = Vector3f(min(mc_min.x, mag.mag.x), min(mc_min.y, mag.mag.y), min(mc_min.z, mag.mag.z));
		mc_max = Vector3f(max(mc_max.x, mag.mag.x), max(mc_max.y, mag.mag.y), max(mc_max.z, mag.mag.z));
		
		Vector3f off = (mc_max + mc_min) / 2;
		
		disp.setCursor(0,80); //DEBUG
		disp.println(off.x);
		disp.println(off.y);
		disp.println(off.z);
		break;
	}
	#endif
}

//=======================Buttons========================
#pragma region Buttons
void buttonPressed(uint8_t button)
{
	// Reset edited value multiplier when any button is pressed (we don't care
	// about center since it exits value editing)
	if (editedValue)
	editedValueIncrement = 1;

	switch (button)
	{
		case BTN_CENTER:
		switch (currentScreen)
		{
			case SCREEN_MAIN:
			changedPreset = currentPreset;
			changingPreset = 1;
			updateCurrentPresetNextLoop = true;
			centerBtnMillis = millis();
			if (settings.centerBtnReload)
			reload();
			return;

			case SCREEN_MAG_CAL:
			settings.magOffset = (mc_max + mc_min) / 2;
			mag.offset = settings.magOffset;
			openMenuListScreen(SCREEN_SETTINGS2);
			return;
		}
		if (isMenuListOpen)
		menuSelect(selectedListItem);
		else if (editedValue)
		stopEditingValue();

		return;

		case BTN_LEFT:
		if (currentScreen == SCREEN_MAIN )
		{
			if(changingPreset){
				do
				{
					if (changedPreset)
					changedPreset--;
					else
					changedPreset = _PRESET_COUNT - 1;
				} while (!settings.presets[changedPreset]);
				updateCurrentPresetNextLoop = true;
				centerBtnMillis = millis();
			}
			else{
				setBrightness(max(brightness - 1, 0));
				updateBrightnessIcon();
			}
		}
		else if (isMenuListOpen && selectedListItem < ScreenToMenuItemCount[currentScreen] - 1)
		{
			switchSelectedListItems(selectedListItem, selectedListItem + 1);
			selectedListItem++;
		}
		return;

		case BTN_RIGHT:
		if (currentScreen == SCREEN_MAIN)
		{
			if(changingPreset)
			{
				do
				{
					if (changedPreset < _PRESET_COUNT - 1)
					changedPreset++;
					else
					changedPreset = 0;
				} while (!settings.presets[changedPreset]);
				updateCurrentPresetNextLoop = true;
				centerBtnMillis = millis();
			}
			else{
				setBrightness(min(brightness + 1, BRIGHTNESS_SETTINGS_COUNT - 1));
				updateBrightnessIcon();
			}
			
		}
		else if (isMenuListOpen && selectedListItem)
		{
			switchSelectedListItems(selectedListItem, selectedListItem - 1);
			selectedListItem--;
		}
		return;

		case BTN_RLD:
		reload();
		return;
	}
}
void buttonHeld(uint8_t button)
{
	if (editedValue && button != BTN_CENTER)
	{
		if (editedValueCtr >= INCREMENT_INTERVAL)
		{
			*editedValue = constrain(*editedValue + (button == BTN_UP ? editedValueIncrement : -editedValueIncrement),
			editedValueMin, editedValueMax);
			changeEditedValue();
			editedValueCtr = 0;
		}
		else
		editedValueCtr += deltaMillis;
		if (((buttonCtrs[BTN_LEFT] > VALUE_EDIT_HOLD_TIME && BTN_LEFT_isPressed) || (buttonCtrs[BTN_RIGHT] > VALUE_EDIT_HOLD_TIME && BTN_RIGHT_isPressed)) && editedValueMax - editedValueMin > 100)
		editedValueIncrement = 10;
	}
}
#pragma endregion
//======================================================

//====================Interrupts====================
#pragma region Interrupts
void reloadInterrupt()
{
	HANDLE_BUTTON_INTERRUPT(BTN_RLD, RELOAD_PINREGISTER, RELOAD_PIN);
}
ISR(PCINT1_vect)
{
	HANDLE_BUTTON_INTERRUPT(BTN_CENTER, PINREGISTER_BTN_CENTER, PIN_BTN_CENTER);
	HANDLE_BUTTON_INTERRUPT(BTN_LEFT, PINREGISTER_BTN_LEFT, PIN_BTN_LEFT);
	HANDLE_BUTTON_INTERRUPT(BTN_RIGHT, PINREGISTER_BTN_RIGHT, PIN_BTN_RIGHT);
}
ISR(PCINT2_vect) {}
ISR(PCINT3_vect)
{
	HANDLE_BUTTON_INTERRUPT(BTN_CENTER, PINREGISTER_BTN_CENTER, PIN_BTN_CENTER);
	HANDLE_BUTTON_INTERRUPT(BTN_LEFT, PINREGISTER_BTN_LEFT, PIN_BTN_LEFT);
	HANDLE_BUTTON_INTERRUPT(BTN_RIGHT, PINREGISTER_BTN_RIGHT, PIN_BTN_RIGHT);
}

void fire()
{
	cli();
	if (bit_is_clear(SENSOR_PINREGISTER, SENSOR_PIN))
	{
		if (!fireDebounce)
		{
			if (settings.presets[currentPreset] < 0){
				ammo++;
				if(ammo > 9999)ammo = 0;
			}
			else
			ammo--;

			// updateAmmo();
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
void fire_delayMeter()
{
	cli();
	if (bit_is_clear(SENSOR_PINREGISTER, SENSOR_PIN))
	{
		if (!fireDebounce)
		{
			unsigned long currentMicros = micros();
			delayMicros = currentMicros - delayMeterMicros;
			delayMeterMicros = currentMicros;
			updateAmmoNextLoop = true;
			fireDebounce = 1;
		}
	}
	else
	{
		fireDebounce = 0;
	}
	sei();
}
void reload()
{
	ammo =
	settings.presets[currentPreset] < 0 ? 0 : settings.presets[currentPreset];
	updateAmmoNextLoop = 2;
}
#pragma endregion
//==================================================

//====================Menu Selects====================
#pragma region Menu Selects

#define SELC(screen, item) case screen * 10 + item:
void menuSelect(uint8_t item)
{
	if (selectionTarget)
	{
		*selectionTarget = selectedListItem;
		selectionTarget = nullptr;
	}
	uint8_t selection = item + currentScreen * 10;
	switch (currentScreen)
	{
		
		default:
		switch (selection)
		{
			//======Settings1:======
			#pragma region
			SELC(SCREEN_SETTINGS1, 0)
			// Save and exit
			saveSettings();
			Rtc.Write(RTC_CALIBRATION, settings.clockCalibration);
			displayMainScreen();
			return;

			SELC(SCREEN_SETTINGS1, 1)
			// Manage presets
			openMenuListScreen(SCREEN_PRESETS);
			disp.setCursor(6, 128 - 12);
			disp.print(F("-1: Count up, 0: OFF"));
			return;

			SELC(SCREEN_SETTINGS1, 2)
			// Set time hours
			editValue(&Rtc.hours, 0, 23);
			queuedTimeSave = true;
			return;

			SELC(SCREEN_SETTINGS1, 3)
			// Set time hours
			editValue(&Rtc.minutes, 0, 59);
			queuedTimeSave = true;
			return;

			SELC(SCREEN_SETTINGS1, 4)
			// UI Setup
			openMenuListScreen(SCREEN_UISETUP);
			return;

			SELC(SCREEN_SETTINGS1, 5)
			// Delay Meter
			openDelayMeter();
			return;

			SELC(SCREEN_SETTINGS1, 6)
			// Save brightness
			eeprom_busy_wait();
			eeprom_update_byte(EA_BRIGHTNESS, brightness);
			disp.setTextColor(settings.bgColor, settings.uiColor); // CC
			disp.setCursor(105, 98);
			disp.print("OK");
			disp.setTextColor(settings.uiColor, settings.bgColor); // RC
			return;

			SELC(SCREEN_SETTINGS1, 7)
			// next page
			openMenuListScreen(SCREEN_SETTINGS2);
			return;
			#pragma endregion
			//======Settings2:======
			#pragma region
			SELC(SCREEN_SETTINGS2, 0)
			// Previous page
			openMenuListScreen(SCREEN_SETTINGS1);
			return;

			SELC(SCREEN_SETTINGS2, 1)
			// Battery selection
			openMenuListScreen(SCREEN_BTSELECT, settings.batteryType);
			selectionTarget = &settings.batteryType;
			return;

			SELC(SCREEN_SETTINGS2, 2)
			// Flip ammo bar direction
			settings.ammoBarDir ^= true;
			disp.setCursor(100, 34);
			disp.setTextColor(settings.bgColor, settings.uiColor);
			disp.print(settings.ammoBarDir ? ">>" : "<<");
			disp.setTextColor(settings.uiColor, settings.bgColor);
			return;

			SELC(SCREEN_SETTINGS2, 3)
			// Toggle empty flash
			settings.emptyFlash ^= true;
			disp.setCursor(100, 50);
			disp.setTextColor(settings.bgColor, settings.uiColor);
			disp.print(settings.emptyFlash ? "ON " : "OFF");
			disp.setTextColor(settings.uiColor, settings.bgColor);
			return;

			SELC(SCREEN_SETTINGS2, 4)
			// Toggle CTR Button Reload
			settings.centerBtnReload = !settings.centerBtnReload;
			disp.setCursor(100, 66);
			disp.setTextColor(settings.bgColor, settings.uiColor); // CC
			disp.print(settings.centerBtnReload ? "ON " : "OFF");
			disp.setTextColor(settings.uiColor, settings.bgColor); // RC
			return;

			SELC(SCREEN_SETTINGS2, 5)
			// Compass Calibration
			disp.fillScreen(settings.bgColor);
			disp.setCursor(0, 0);
			disp.print(F("Slowly rotate the\ndevice around every\naxis\n\nPress [SEL] to exit\n\nCurrent Offset:"));
			mc_min = Vector3f(INT16_MAX, INT16_MAX, INT16_MAX);
			mc_max = Vector3f(INT16_MIN, INT16_MIN, INT16_MIN);
			currentScreen = SCREEN_MAG_CAL;
			return;

			SELC(SCREEN_SETTINGS2, 6)
			// Compass mode
			openMenuListScreen(SCREEN_COMPASS_SETTINGS, settings.compassMode);
			selectionTarget = &settings.compassMode;
			return;
			
			SELC(SCREEN_SETTINGS2, 7)
			// next page
			openMenuListScreen(SCREEN_SETTINGS3);
			return;
			#pragma endregion
			
			//======Settings3:======
			#pragma region
			SELC(SCREEN_SETTINGS3, 0)
			// Previous page
			openMenuListScreen(SCREEN_SETTINGS2);
			return;
			
			SELC(SCREEN_SETTINGS3,1)
			// Toggle HopUp Assist Mode
			settings.hAssistEnabled = !settings.hAssistEnabled;
			disp.setCursor(100,LIST_ITEM_Y * 1);
			disp.setTextColor(settings.bgColor, settings.uiColor);
			disp.print(settings.hAssistEnabled ? "ON " : "OFF");
			disp.setTextColor(settings.uiColor, settings.bgColor);
			return;
			
			SELC(SCREEN_SETTINGS3, 2)
			zeroHA();
			return;
			
			SELC(SCREEN_SETTINGS3, 3)
			// Clock Calibration
			editValue(&settings.clockCalibration,INT8_MIN,INT8_MAX);
			return;
			
			SELC(SCREEN_SETTINGS3, 4)
			// Factory reset
			factoryResetCtr++;
			if (factoryResetCtr >= FACTORY_RESET_CONFIRMATIONS)
			{
				factoryReset();
				displayMainScreen();
			}
			else
			{
				disp.setCursor(100, LIST_ITEM_Y * 4);
				disp.setTextColor(settings.bgColor, settings.uiColor); // CC
				disp.print('>');
				disp.print(FACTORY_RESET_CONFIRMATIONS - factoryResetCtr);
				disp.setTextColor(settings.uiColor, settings.bgColor); // RC
			}
			return;
			#pragma endregion
			
			
			//======UI Setup:======
			#pragma region
			SELC(SCREEN_UISETUP, 0)
			openMenuListScreen(SCREEN_SETTINGS1);
			return;

			SELC(SCREEN_UISETUP, 2)
			SELC(SCREEN_UISETUP, 3)
			SELC(SCREEN_UISETUP, 4)
			SELC(SCREEN_UISETUP, 5)
			SELC(SCREEN_UISETUP, 6)
			editedColor = &settings.uiColor + item - 2;
			editedColorComponents[0] = GET_RED(*editedColor);
			editedColorComponents[1] = GET_GREEN(*editedColor);
			editedColorComponents[2] = GET_BLUE(*editedColor);
			openMenuListScreen(SCREEN_COLOR);
			return;

			SELC(SCREEN_UISETUP, 1)
			settings.rotation =
			settings.rotation >= 3 ? 0 : settings.rotation + 1;
			disp.setRotation(settings.rotation);
			openMenuListScreen(SCREEN_UISETUP, 1);
			return;
			
			SELC(SCREEN_UISETUP, 7)
			// Select ammo bar style
			openMenuListScreen(SCREEN_AMMOBARSELECT, settings.selectedAmmoBar);
			selectionTarget = &settings.selectedAmmoBar;
			return;
		}
		#pragma endregion
		return;

		case SCREEN_PRESETS:
		if (!item)
		{
			openMenuListScreen(SCREEN_SETTINGS1);
			return;
		}
		editValue(settings.presets + item - 1, -1, 9999);
		return;

		case SCREEN_AMMOBARSELECT:
		openMenuListScreen(SCREEN_UISETUP);
		return;
		
		case SCREEN_DELAYMETER:
		attachInterrupt(SENSOR_INTERRUPT, &fire, CHANGE);
		openMenuListScreen(SCREEN_SETTINGS1);
		return;
		
		case SCREEN_COMPASS_SETTINGS:
		case SCREEN_BTSELECT:
		openMenuListScreen(SCREEN_SETTINGS2);
		return;

		case SCREEN_COLOR:
		if (!item)
		{
			*editedColor = editedColorComponents[0] << SHIFT_RED |
			editedColorComponents[1] << SHIFT_GREEN |
			editedColorComponents[2];
			editedColor = nullptr;
			openMenuListScreen(SCREEN_UISETUP);
			return;
		}
		editValue(editedColorComponents + item - 1, 0, item == 2 ? 63 : 31);
		return;
	}
}
#pragma endregion
//====================================================

//====================Display functions====================
#pragma region Display functions
void openMenuListScreen(uint8_t screen, uint8_t selectedItem)
{
	currentScreen = screen;
	disp.fillScreen(settings.bgColor);

	// Draw list menu
	const char **contents = ScreenToMenuContents[screen];
	uint8_t itemCount = ScreenToMenuItemCount[screen];

	for (uint8_t i = 0; i < itemCount; i++)
	{
		if (i == selectedItem)
		{
			disp.fillRect(LIST_MENU_X, LIST_MENU_Y + (LIST_MENU_H + LIST_MENU_S) * i, LIST_MENU_W, LIST_MENU_H, settings.uiColor);
			disp.setTextColor(settings.bgColor, settings.uiColor);
		}
		else
		disp.drawRect(LIST_MENU_X, LIST_MENU_Y + (LIST_MENU_H + LIST_MENU_S) * i, LIST_MENU_W, LIST_MENU_H, settings.uiColor);

		disp.setCursor(LIST_MENU_X + 2,	LIST_MENU_Y + 2 + (LIST_MENU_H + LIST_MENU_S) * i);
		disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(contents + i));
		disp.setTextColor(settings.uiColor, settings.bgColor);
	}
	selectedListItem = selectedItem;
	isMenuListOpen = true;

	if (screen == SCREEN_COLOR)
	disp.fillRect((128 - 26) / 2, 128 * 3 / 4, 26, 26, editedColorComponents[0] << SHIFT_RED | editedColorComponents[1] << SHIFT_GREEN | editedColorComponents[2]);
}
/*
void openSettingsScreen()
{
currentScreen = SCREEN_SETTINGS1;
disp.fillScreen(settings.bgColor);
//disp.setTextColor(settings.uiColor);//potential error
disp.setCursor(40,0);
disp.print("Settings");
openMenuListScreen(
}

void openSettingsScreen2()
{
openMenuListScreen(SCREEN_SETTINGS2);
disp.setCursor(5,116);
disp.print(F("Version: "));
//disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&str));
disp.print(VERSION);
}*/
void printTime()
{
	if (Rtc.hours < 10)
	disp.print('0');
	disp.print(Rtc.hours);
	disp.print(':');
	if (Rtc.minutes < 10)
	disp.print('0');
	disp.print(Rtc.minutes);
}

void switchSelectedListItems(uint8_t deselectedItem, uint8_t selectedItem)
{
	const char **contents = ScreenToMenuContents[currentScreen];
	disp.fillRect(LIST_MENU_X + 1,
	LIST_MENU_Y + 1 + (LIST_MENU_H + LIST_MENU_S) * deselectedItem,
	LIST_MENU_W - 2, LIST_MENU_H - 2, settings.bgColor);
	disp.setCursor(LIST_MENU_X + 2,
	LIST_MENU_Y + 2 +
	(LIST_MENU_H + LIST_MENU_S) * deselectedItem);
	disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(
	contents + deselectedItem));

	disp.setTextColor(settings.bgColor, settings.uiColor);
	disp.fillRect(LIST_MENU_X + 1,
	LIST_MENU_Y + 1 + (LIST_MENU_H + LIST_MENU_S) * selectedItem,
	LIST_MENU_W - 2, LIST_MENU_H - 2, settings.uiColor);
	disp.setCursor(LIST_MENU_X + 2,
	LIST_MENU_Y + 2 + (LIST_MENU_H + LIST_MENU_S) * selectedItem);
	disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(
	contents + selectedItem));
	disp.setTextColor(settings.uiColor, settings.bgColor);
}

void openDelayMeter()
{
	currentScreen = SCREEN_DELAYMETER;
	attachInterrupt(SENSOR_INTERRUPT, &fire_delayMeter, CHANGE);
	disp.fillScreen(settings.bgColor);
	disp.setCursor(0,20);
	disp.print(F("Delay ms:\n\nRPM:\n\nPress [SEL] to exit"));
}
#pragma endregion
//=========================================================

//====================Main Screen Functions====================
#pragma region Main Screen Functions
void displayMainScreen()
{
	isMenuListOpen = false;
	currentScreen = SCREEN_MAIN;
	disp.fillScreen(settings.bgColor);
	// disp.setTextColor(settings.uiColor,settings.bgColor);
	disp.drawBitmap(88, 112, IconBattery, settings.uiColor, settings.bgColor);
	disp.drawBitmap(37, 112, IconClock, settings.uiColor, settings.bgColor);
	#ifdef HAS_MAGNETOMETER
	if(settings.compassMode)
	disp.drawBitmap(COMPASS_TXT_X - 16, COMPASS_TXT_Y +1, IconCompass, settings.uiColor, settings.bgColor);
	if(settings.hAssistEnabled & 1)
	disp.drawBitmap(128 - HA_START_X - 7,HA_START_Y,IconUP, settings.statusColors[2],settings.bgColor);
	lastHAssistColor = false;
	#endif
	updateAmmoBar();
	updateAmmoNextLoop = true;
	updateCurrentPreset();
	factoryResetCtr = 0;
	updateBrightnessIcon();
	
}

void updateAmmoBar()
{
	uint8_t size;
	if (settings.presets[currentPreset] < 0)
	{
		size = 0;
		ammoColor = settings.statusColors[0];
	}
	else
	{
		size =
		ammo > 0 ? map(ammo, 0, settings.presets[currentPreset], 0, 128) : 0;

		if (size > 85)
		ammoColor = settings.statusColors[0];
		else if (size > 42)
		ammoColor = settings.statusColors[1];
		else
		ammoColor = settings.statusColors[2];
	}
	drawAmmoBar(size);
}

void drawAmmoBar(uint8_t size)
{
	if (size == 0)
	{
		disp.fillRect(0, 8, 128, 20, settings.bgColor);
		return;
	}
	int16_t crop = 128 - size;
	if(settings.selectedAmmoBar)
	disp.drawBitmap(0,8, AmmoBars[settings.selectedAmmoBar - 1], ammoColor,	settings.bgColor, settings.ammoBarDir ? -crop : crop, 0);
	else
	disp.fillCroppedRect(0,8,128,20,ammoColor,settings.bgColor,settings.ammoBarDir ? -crop : crop, 0);
}
void setBrightness(uint8_t newBrightness)
{
	brightness = newBrightness;
	analogWrite(DSP_BL, BRIGHTNESS_LEVELS[brightness]);
}


void updateBrightnessIcon()
{
	disp.drawBitmap(BRIGHTNESS_ICON_X,BRIGHTNESS_ICON_Y, IconBrightness, settings.uiColor,settings.bgColor,( BRIGHTNESS_ICON_SIZE / BRIGHTNESS_SETTINGS_COUNT * (BRIGHTNESS_SETTINGS_COUNT-1 - brightness)),0);
}

void updateTime()
{
	disp.setCursor(6, 112);
	if (queuedTimeSave)
	{
		disp.print("FAIL");
		disp.setCursor(6, 112);
		Rtc.SetTime();
		queuedTimeSave = 0;
	}
	else
	Rtc.Update();
	printTime();
	battMeasurementMillis = millis();
}

void updateBattery()
{
	batteryBuffer.add(analogRead(BATTERY_ADC_CHANNEL));

	float voltage = (batteryBuffer.get() * 11 / 1024.0f) * 1.127f;

	disp.setCursor(102, 112);
	//	disp.setFont();
	// disp.setTextSize(1);
	// disp.setTextColor(settings.uiColor,settings.bgColor);

	if (settings.batteryType)
	{
		uint8_t cells = 1;
		switch (settings.batteryType)
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
		switch (settings.batteryType)
		{
			case BT_NIMH_7CELL:
			case BT_NIMH_8CELL:
			batteryPercentage = calculateBatteryPrecentage(
			voltage / cells, NIMH_H_OFFSET, NIMH_K, NIMH_BASE, NIMH_V_OFFSET);
			break;

			case BT_LIPO_7V4:
			case BT_LIPO_11V1:
			batteryPercentage = calculateBatteryPrecentage(
			voltage / cells, LIPO_H_OFFSET, LIPO_K, LIPO_BASE, LIPO_V_OFFSET);
			break;

			case BT_StA_9VOLT:
			batteryPercentage = max(0, min(100, 100.0f * (voltage - StA_9V_MIN) /
			(StA_9V_MAX - StA_9V_MIN)));
			break;
		}
		batteryPercentage =
		map(batteryPercentage, settings.batteryCalibration, 100, 0, 100);
		if (batteryPercentage < 0)
		batteryPercentage = 0;
		disp.setTextColor(batteryPercentage < 15 ?  settings.statusColors[2] : settings.statusColors[0], settings.bgColor); // change color = CC
		disp.print(batteryPercentage, DEC);
		disp.print('%');
		disp.setTextColor(settings.uiColor,	settings.bgColor); // revert change = RC
		disp.fillRect(disp.getCursorX(), disp.getCursorY(), 5, 7,
		settings.bgColor);
	}
	else
	disp.print(voltage, 1); // Raw voltage
}

void updateCurrentPreset()
{
	// disp.setTextColor(settings.uiColor, settings.bgColor);
	disp.setCursor(59, 109);
	disp.setTextSize(2);
	disp.print(changingPreset ? changedPreset + 1 : currentPreset + 1);
	disp.setTextSize(1);

	disp.setTextColor(changingPreset ? settings.uiColor : settings.bgColor); // CC
	// uint8_t orgCursorX =  disp.getCursorX();
	// uint8_t orgCursorY =  disp.getCursorY();
	disp.setCursor(49, 109);
	disp.print('<');
	disp.setCursor(73, 109);
	disp.print('>');
	// disp.setCursor(orgCursorX,orgCursorY);
	disp.setTextColor(settings.uiColor, settings.bgColor); // RC
}

void updateHAssist(){
	Vector3f front = Vector3f(1, 0, 0);
	Vector3f g_f_plane_normal = Vector3f::CrossProduct(mag.acc, front);
	g_f_plane_normal.Normalize();
	
	float haDelta = Vector3f::DotProduct(settings.hAssistBaseRight, g_f_plane_normal);
	bool newColor = haDelta >= HA_THRESHOLD;
	if(settings.hAssistEnabled & 1 && newColor != lastHAssistColor){
		disp.drawBitmap(128 - HA_START_X - 7,HA_START_Y,IconUP,newColor ?  settings.statusColors[0]: settings.statusColors[2],settings.bgColor);
		lastHAssistColor = newColor;
	}
}

void updateCompass()
{
	disp.setCursor(COMPASS_TXT_X, COMPASS_TXT_Y);
	disp.setTextSize(2);
	int bearing = mag.getMagneticBearing();
	
	if(settings.compassMode == 1){
		disp.print(bearing);
		if(bearing < 100)
		disp.print(' ');
		if(bearing < 10)
		disp.print(' ');
	}
	else
	{
		void* label = settings.compassMode == 2 ? (CardinalDirections + bearing / 45) : (CardinalDirectionsExt + ((int)(bearing / 22.5F)));
		disp.print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(label));
	}
	disp.setTextSize(1);
}
#pragma endregion
//=============================================================

//====================Math====================
#pragma region Math Functions
uint8_t calculateBatteryPrecentage(float voltage, float h_offset, float k, float base, float v_offset)
{
	float percent = (1 / (pow(base, h_offset - voltage / k) + 1)) + v_offset;
	return min(100, (uint8_t)(100 * percent));
}
#pragma endregion
//============================================

//====================System functions====================
#pragma region System functions

void editValue(int *value, int _min, int _max)
{
	editedValue = value;
	editedValueMin = _min;
	editedValueMax = _max;

	isMenuListOpen = false;
	previousScreen = currentScreen;
	currentScreen = SCREEN_EDITVALUE;
	disp.drawRect(20, 44, 128 - 20 * 2, 40, settings.uiColor);
	disp.fillRect(21, 45, 128 - 21 * 2, 38, settings.bgColor);
	disp.setCursor(EDITVALUE_MSG_X, EDITVALUE_MSG_Y);
	disp.print(F("Edit value:"));
	disp.drawRect(22, EDITVALUE_BAR_Y, 128 - 22 * 2, 5, settings.uiColor);
	changeEditedValue();
}
void changeEditedValue()
{
	disp.setCursor(EDITVALUE_VAL_X, EDITVALUE_VAL_Y);
	disp.print(*editedValue);
	disp.print("    ");
	disp.fillCroppedRect(23, EDITVALUE_BAR_Y + 1, 128 - 23 * 2, 3, settings.bgColor, settings.uiColor, -map(*editedValue, editedValueMin, editedValueMax, 0, 128 - 23 * 2), 0);
}
inline void stopEditingValue()
{
	editedValue = nullptr;
	openMenuListScreen(previousScreen, selectedListItem);
}

void zeroHA()
{
	mag.read3Axis(ACC);
	// Set HopUp Assist (HA) Zero
	Vector3f nacc = mag.acc;
	nacc.Normalize();
	Vector3f right = Vector3f::CrossProduct(nacc,Vector3f(1, 0, 0));
	right.Normalize();
	settings.hAssistBaseRight = right;
	disp.setCursor(100, LIST_ITEM_Y * 2);
	disp.setTextColor(settings.bgColor,settings.uiColor);
	disp.print("OK");
	disp.setTextColor(settings.uiColor,settings.bgColor);
}

void calibrateBattery() { settings.batteryCalibration = batteryPercentage; }

void loadSettings()
{
	eeprom_busy_wait();
	uint8_t vtag = eeprom_read_byte(0x00);
	if (vtag != _EEPROM_VERIFY_TAG)
	{
		factoryReset();
		return;
	}

	brightness = eeprom_read_byte(EA_BRIGHTNESS);

	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		((uint8_t *)(&settings))[i] = eeprom_read_byte(EA_SETTINGS + i);
	}
}
void saveSettings()
{
	eeprom_busy_wait();
	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		eeprom_update_byte(EA_SETTINGS + i, ((uint8_t *)(&settings))[i]);
	}
}
void factoryReset()
{
	eeprom_busy_wait();
	eeprom_write_byte(0x00, _EEPROM_VERIFY_TAG);
	eeprom_write_byte(EA_BRIGHTNESS, FACT_BRIGHTNESS);

	__TachyonSettings factorySettings;
	factorySettings.uiColor = FACT_UICOLOR;
	factorySettings.bgColor = ST7735_BLACK;
	factorySettings.statusColors[0] = FACT_CTRCOLOR1;
	factorySettings.statusColors[1] = FACT_CTRCOLOR2;
	factorySettings.statusColors[2] = FACT_CTRCOLOR3;

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
	factorySettings.batteryCalibration = FACT_BATTERYCALIBRATION;
	
	#ifdef HAS_MAGNETOMETER
	factorySettings.magOffset = Vector3f();
	factorySettings.hAssistEnabled = FACT_HA_MODE;
	factorySettings.compassMode = FACT_COMPASS_MODE;
	factorySettings.hAssistBaseRight = Vector3f(0,1,0);
	#endif
	
	size_t s = sizeof(settings);
	for (size_t i = 0; i < s; i++)
	{
		eeprom_write_byte(EA_SETTINGS + i, ((uint8_t *)&factorySettings)[i]);
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
