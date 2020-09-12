/*===============================================================
This library is a heavily modified and optimized version of the Adafruit ST7735 library
Only suitable for Tachyon Weapon Computer. Unmodified version(s) of the original library are NOT COMPATIBLE!
https://github.com/adafruit/Adafruit-ST7735-Library

Modified by Martin Hrehor
=================================================================
*/

//Original library licensing info:
/***************************************************
This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
----> http://www.adafruit.com/products/358
The 1.8" TFT shield
----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
----> http://www.adafruit.com/products/618

Check out the links above for our tutorials and wiring diagrams
These displays use SPI to communicate, 4 or 5 pins are required to
interface (RST is optional)
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution
****************************************************/

#ifndef _ADAFRUIT_ST7735H_
#define _ADAFRUIT_ST7735H_

#include "Arduino.h"
#include "Print.h"
//#include <Adafruit_GFX.h>
#include "../Adafruit-GFX/Adafruit_GFX.h"

#if defined(__AVR__) || defined(CORE_TEENSY)
#include <avr/pgmspace.h>
#define USE_FAST_IO
typedef volatile uint8_t RwReg;
#elif defined(ARDUINO_STM32_FEATHER)
typedef volatile uint32 RwReg;
#define USE_FAST_IO
#elif defined(ARDUINO_FEATHER52)
typedef volatile uint32_t RwReg;
#define USE_FAST_IO
#elif defined(ESP8266)
#include <pgmspace.h>
#elif defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#include <include/pio.h>
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
typedef unsigned char prog_uchar;
#endif


#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 128


#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_ORANGE  0xFBE0

class Adafruit_ST7735 : public Adafruit_GFX {

	public:

	Adafruit_ST7735(int8_t CS, int8_t RS, int8_t SID, int8_t SCLK, int8_t RST = -1);
	Adafruit_ST7735(int8_t CS, int8_t RS, int8_t RST = -1);

	void
	init(),                         
	setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1),
	pushColor(uint16_t color),
	fillScreen(uint16_t color),
	drawPixel(int16_t x, int16_t y, uint16_t color),
	drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint16_t color),
	drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint16_t color),
	fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color),
	fillCroppedRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color,uint16_t bg, int16_t cropX, int16_t cropY),
	setRotation(uint8_t r),
	invertDisplay(boolean i);
	uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);

	void drawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint16_t color, uint16_t bg);
	//Displays a bitmap that is cropped on the side(s), with the cropped area being filled with bgcolor
	void drawBitmap(uint8_t x, uint8_t y, const uint8_t bitmap[], uint16_t color, uint16_t bg, int16_t cropX, int16_t cropY);
	//Displays a bitmap that is cropped on the side(s), with the cropped area being filled with bgcolor, and is cut off at the specified width/height (same as cropping, but the area is left alone insead of being filled with bgcolor
	//void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], uint16_t color, uint16_t bg, int16_t cropX, int16_t cropY,int16_t cutoffX, int16_t cutoffY);

	inline void CS_HIGH(void);
	inline void CS_LOW(void);
	 void DC_HIGH(void);
	 void DC_LOW(void);

	private:
	void writecommand(uint8_t c),
	writedata(uint8_t d),
	commandList(const uint8_t *addr);




	int8_t  _cs, _dc, _rst, _sid, _sclk;
	uint8_t colstart, rowstart, xstart, ystart; // some displays need this changed

	#if defined(USE_FAST_IO)
	volatile RwReg  *dataport, *clkport, *csport, *dcport;

	#if defined(__AVR__) || defined(CORE_TEENSY)  // 8 bit!
	uint8_t  datapinmask, clkpinmask, cspinmask, dcpinmask;
	#else    // 32 bit!
	uint32_t  datapinmask, clkpinmask, cspinmask, dcpinmask;
	#endif
	#endif

};


#endif
