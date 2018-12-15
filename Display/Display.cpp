/*===============================================================
This library is a heavily modified and optimized version of the Adafruit ST7735 library
Only suitable for Tachyon WCS V1. Unmodified version(s) of the original library are NOT COMPATIBLE!
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

#include "Display.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include "../Conf.h"
#include <SPI.h>

inline uint16_t swapcolor(uint16_t x) {
	return (x << 11) | (x & 0x07E0) | (x >> 11);
}
#undef SPI_HAS_TRANSACTION
#if defined (SPI_HAS_TRANSACTION)
static SPISettings mySPISettings;
#elif defined (__AVR__) || defined(CORE_TEENSY)
/*static uint8_t SPCRbackup;
static uint8_t mySPCR;*/
#endif

//Returns the sign bit (1 of number is negative, 0 if positive)
#define sign8(x) (x & 0x80)
#define sign16(x) (x & 0x8000)

// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t dc, int8_t sid, int8_t sclk, int8_t rst)
: Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT)
{
	_cs   = cs;
	_dc   = dc;
	_sid  = sid;
	_sclk = sclk;
	_rst  = rst;
	//hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t dc, int8_t rst)
: Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT) {
	_cs   = cs;
	_dc   = dc;
	_rst  = rst;
	//hwSPI = true;
	_sid  = _sclk = -1;
}
/*
inline void Adafruit_ST7735::spiwrite(uint8_t c) {
SPI.transfer(c);
}*/


void Adafruit_ST7735::writecommand(uint8_t c) {
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif

	CS_LOW();
	DC_LOW();
	SPI.transfer(c);

	CS_HIGH();
	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}


void Adafruit_ST7735::writedata(uint8_t c) {
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif

	CS_LOW();
	DC_HIGH();
	SPI.transfer(c);

	CS_HIGH();
	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t PROGMEM CmdList[] =
{
	21,                       // 21 commands in list:
	ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
	150,                    //     150 ms delay
	ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
	250,                    //     250 ms delay
	ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
	0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
	0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
	0x01, 0x2C, 0x2D,       //     Dot inversion mode
	0x01, 0x2C, 0x2D,       //     Line inversion mode
	ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
	0x07,                   //     No inversion
	ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
	0xA2,
	0x02,                   //     -4.6V
	0x84,                   //     AUTO mode
	ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
	0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
	ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
	0x0A,                   //     Opamp current small
	0x00,                   //     Boost frequency
	ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
	0x8A,                   //     BCLK/2, Opamp current small & Medium low
	0x2A,
	ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
	0x8A, 0xEE,
	ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
	0x0E,
	ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
	ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
	0xC8,                   //     row addr/col addr, bottom to top refresh
	ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
	0x05,					//     16-bit color

	ST7735_CASET  , 4      ,  //  16: Column addr set, 4 args, no delay:
	0x00, 0x00,             //     XSTART = 0
	0x00, 0x7F,             //     XEND = 127
	ST7735_RASET  , 4      ,  //  17: Row addr set, 4 args, no delay:
	0x00, 0x00,             //     XSTART = 0
	0x00, 0x7F,				//     XEND = 127

	ST7735_GMCTRP1, 16      , //  18: Gamma correction positive, 16 args, no delay:
	0x02, 0x1c, 0x07, 0x12,
	0x37, 0x32, 0x29, 0x2d,
	0x29, 0x25, 0x2B, 0x39,
	0x00, 0x01, 0x03, 0x10,
	ST7735_GMCTRN1, 16      , //  19: Gamma correction negative, 16 args, no delay:
	0x03, 0x1d, 0x07, 0x06,
	0x2E, 0x2C, 0x29, 0x2D,
	0x2E, 0x2E, 0x37, 0x3F,
	0x00, 0x00, 0x02, 0x10,
	
	ST7735_NORON  ,    DELAY, //  20: Normal display on, no args, w/delay
	10,                     //     10 ms delay
	ST7735_DISPON ,    DELAY, //  21: Main screen turn on, no args w/delay
	100						//     100 ms delay
};


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST7735::commandList(const uint8_t *addr) {

	uint8_t  numCommands, numArgs;
	uint16_t ms;

	numCommands = pgm_read_byte(addr++);   // Number of commands to follow
	while(numCommands--) {                 // For each command...
		writecommand(pgm_read_byte(addr++)); //   Read, issue command
		numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
		ms       = numArgs & DELAY;          //   If hibit set, delay follows args
		numArgs &= ~DELAY;                   //   Mask out delay bit
		while(numArgs--) {                   //   For each argument...
			writedata(pgm_read_byte(addr++));  //     Read, issue argument
		}

		if(ms) {
			ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
			if(ms == 255) ms = 500;     // If 255, delay for 500 ms
			delay(ms);
		}
	}
}

void Adafruit_ST7735::init()
{
	ystart = xstart = 0;
	_height = ST7735_TFTHEIGHT;
	_width  = ST7735_TFTWIDTH;

	pinMode(_dc, OUTPUT);
	pinMode(_cs, OUTPUT);

	#if defined(USE_FAST_IO)
	csport    = portOutputRegister(digitalPinToPort(_cs));
	dcport    = portOutputRegister(digitalPinToPort(_dc));
	cspinmask = digitalPinToBitMask(_cs);
	dcpinmask = digitalPinToBitMask(_dc);
	#endif

	#if defined (SPI_HAS_TRANSACTION)
	SPI.begin();
	mySPISettings = SPISettings(8000000, MSBFIRST, SPI_MODE0);
	#elif defined (__AVR__) || defined(CORE_TEENSY)
	//SPCRbackup = SPCR;
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV4);
	SPI.setDataMode(SPI_MODE0);
	#elif defined (__SAM3X8E__)
	SPI.begin();
	SPI.setClockDivider(21); //4MHz
	SPI.setDataMode(SPI_MODE0);
	#endif

	// toggle RST low to reset; CS low so it'll listen to us
	/*CS_LOW();
	if (_rst != -1) {
	pinMode(_rst, OUTPUT);
	digitalWrite(_rst, HIGH);
	delay(500);
	digitalWrite(_rst, LOW);
	delay(500);
	digitalWrite(_rst, HIGH);
	delay(500);
	}*/

	colstart = 2;
	rowstart = 3;
	commandList(CmdList);
}

void Adafruit_ST7735::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,
uint8_t y1) {

	writecommand(ST7735_CASET); // Column addr set
	writedata(0x00);
	writedata(x0+xstart);     // XSTART
	writedata(0x00);
	writedata(x1+xstart);     // XEND

	writecommand(ST7735_RASET); // Row addr set
	writedata(0x00);
	writedata(y0+ystart);     // YSTART
	writedata(0x00);
	writedata(y1+ystart);     // YEND

	writecommand(ST7735_RAMWR); // write to RAM
}


void Adafruit_ST7735::pushColor(uint16_t color) {
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif


	CS_LOW();
	DC_HIGH();
	SPI.transfer(color >> 8);
	DC_HIGH();
	SPI.transfer(color);
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}

void Adafruit_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

	setAddrWindow(x,y,x+1,y+1);

	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif


	CS_LOW();
	DC_HIGH();
	SPI.transfer(color >> 8);
	DC_HIGH();
	SPI.transfer(color);
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}


void Adafruit_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h,
uint16_t color) {

	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height) h = _height-y;
	setAddrWindow(x, y, x, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif


	CS_LOW();

	while (h--) {
		DC_HIGH();
		SPI.transfer(hi);
		DC_HIGH();
		SPI.transfer(lo);
	}
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}


void Adafruit_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,
uint16_t color) {

	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
	setAddrWindow(x, y, x+w-1, y);

	uint8_t hi = color >> 8, lo = color;

	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif

	CS_LOW();
	while (w--) {
		DC_HIGH();
		SPI.transfer(hi);
		DC_HIGH();
		SPI.transfer(lo);
	}
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}



void Adafruit_ST7735::fillScreen(uint16_t color) {
	fillRect(0, 0,  _width, _height, color);
}



// fill a rectangle
void Adafruit_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
uint16_t color) {

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	setAddrWindow(x, y, x+w-1, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif

	CS_LOW();
	
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			DC_HIGH();
			SPI.transfer(hi);
			DC_HIGH();
			SPI.transfer(lo);
		}
	}
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}

// takes up ~500 bytes, can be optimized

void Adafruit_ST7735::fillCroppedRect(int16_t x, int16_t y, int16_t w, int16_t h,
uint16_t color,uint16_t bg, int16_t cropX, int16_t cropY) {

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	setAddrWindow(x, y, x+w-1, y+h-1);

	uint8_t hi = color >> 8, lo = color, bgH = bg >> 8, bgL = bg;
	
	#if defined (SPI_HAS_TRANSACTION)
	SPI.beginTransaction(mySPISettings);
	#endif

	CS_LOW();
	uint8_t xPos = 0, yPos=0;
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			bool b = ((sign16(cropX) && xPos >= -cropX) || (!sign16(cropX) && xPos <= w-cropX-1)) && ((sign16(cropY) && yPos >= -cropY) || (!sign16(cropY) && yPos <= h-cropY-1));
			DC_HIGH();
			SPI.transfer(b? hi : bgH);
			DC_HIGH();
			SPI.transfer(b? lo : bgL);
			xPos++;
		}
		xPos = 0;
		yPos++;
	}
	CS_HIGH();

	#if defined (SPI_HAS_TRANSACTION)
	SPI.endTransaction();
	#endif
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void Adafruit_ST7735::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], uint16_t color, uint16_t bg)
{
	uint8_t w,  h;
	w = pgm_read_byte(&bitmap[0]);
	h = pgm_read_byte(&bitmap[1]);

	setAddrWindow(x, y, x+w-1, y+h-1);

	uint8_t xPos = 0, yPos = 0, bitPos = 0;
	uint8_t byte = pgm_read_byte(&bitmap[2]);

	uint8_t cH = color >> 8,
	cL = color,
	bgH = bg >> 8,
	bgL = bg;
	
	uint16_t size = w * h;
	
	CS_LOW();
	for(uint16_t i = 0; i < size; i++)
	{
		bool b = byte & (0x80 >> bitPos);

		DC_HIGH();
		SPI.transfer(b? cH: bgH);
		DC_HIGH();
		SPI.transfer(b? cL : bgL);


		if(bitPos < 7)bitPos++;
		else
		{
			bitPos = 0;
			byte = pgm_read_byte(&bitmap[((i+1)/8) + 2]);
		}

		if(xPos < w -1 )xPos++;
		else{ xPos = 0; yPos++;}
	}
	CS_HIGH();
}


void Adafruit_ST7735::drawBitmap(int16_t x_in, int16_t y_in, const uint8_t bitmap[], uint16_t color, uint16_t bg, int16_t cropX, int16_t cropY)
{
	uint8_t w, h, cropLeft, cropRight, cropTop, cropBottom;
	w = pgm_read_byte(&bitmap[0]);
	h = pgm_read_byte(&bitmap[1]);

	if(sign16(cropX)){
		cropLeft = -cropX;
		cropRight = 0;
	}
	else{
		cropRight = cropX;
		cropLeft = 0;
	}
	if(sign16(cropY)){
		cropTop = -cropY;
		cropBottom = 0;
	}
	else{
		cropBottom = cropY;
		cropTop = 0;
	}

	setAddrWindow(x_in, y_in, x_in+w-1, y_in+h-1);
	
	uint8_t xPos = 0, yPos = 0, bitPos = 0;
	uint8_t byte = pgm_read_byte(&bitmap[2]);

	uint8_t cH = color >> 8,
	cL = color,
	bgH = bg >> 8,
	bgL = bg;

	uint16_t size = w * h;
	CS_LOW();
	for(uint16_t i = 0; i < size; i++)
	{
		bool b = (byte & (0x80 >> bitPos)) && !(xPos < cropLeft || xPos >= w-cropRight || yPos < cropTop || yPos >= h-cropBottom);
		DC_HIGH();
		SPI.transfer(b? cH : bgH);
		DC_HIGH();
		SPI.transfer(b? cL : bgL);

		if(bitPos < 7)bitPos++;
		else
		{
			bitPos = 0;
			byte = pgm_read_byte(&bitmap[((i+1)/8) + 2]);
		}

		if(xPos < w -1 )xPos++;
		else{ xPos = 0; yPos++;}
	}
	CS_HIGH();
}

/*
void Adafruit_ST7735::drawBitmap(int16_t x_in, int16_t y_in, const uint8_t bitmap[], uint16_t color, uint16_t bg, int16_t cropX, int16_t cropY,int16_t cutoffX, int16_t cutoffY)
{
	uint8_t w, h, x, y, cropLeft, cropRight, cropTop, cropBottom, cutoffLeft, cutoffRight, cutoffTop, cutoffBottom, w_cutoff, h_cutoff;
	w = pgm_read_byte(&bitmap[0]);
	h = pgm_read_byte(&bitmap[1]);

	w_cutoff = w - abs(cutoffX);
	h_cutoff = h - abs(cutoffY);

	if(sign16(cutoffX)){
		cutoffLeft = -cutoffX;
		cutoffRight = 0;
	}
	else{
		cutoffRight = cutoffX;
		cutoffLeft = 0;
	}
	if(sign16(cutoffY)){
		cutoffTop = -cutoffY;
		cutoffBottom = 0;
	}
	else{
		cutoffBottom = cutoffY;
		cutoffTop = 0;
	}


	if(sign16(cropX)){
		cropLeft = -cropX;
		cropRight = 0;
	}
	else{
		cropRight = cropX;
		cropLeft = 0;
	}
	if(sign16(cropY)){
		cropTop = -cropY;
		cropBottom = 0;
	}
	else{
		cropBottom = cropY;
		cropTop = 0;
	}


	x = x_in;// - cutoffLeft;
	y = y_in;// - cutoffTop;

	setAddrWindow(x, y, x+w_cutoff-1, y+h_cutoff-1);

	uint8_t xPos = 0, yPos = 0, bitPos = 0;
	uint8_t byte = pgm_read_byte(&bitmap[2]);

	uint8_t cH = color >> 8,
	cL = color,
	bgH = bg >> 8,
	bgL = bg;

	uint16_t size = w * h;
	CS_LOW();
	for(uint16_t i = 0; i < size; i++)
	{
		if(xPos >= cutoffLeft && xPos < w-cutoffRight && yPos >= cutoffTop && yPos < h-cutoffBottom){
			bool b = (byte & (0x80 >> bitPos)) && !(xPos < cropLeft+cutoffLeft || xPos >= w-cropRight-cutoffRight || yPos < cropTop+cutoffTop || yPos >= h-cropBottom-cutoffBottom);
			DC_HIGH();
			SPI.transfer(b? cH : bgH);
			DC_HIGH();
			SPI.transfer(b? cL : bgL);
		}

		if(bitPos < 7)bitPos++;
		else
		{
			bitPos = 0;
			byte = pgm_read_byte(&bitmap[((i+1)/8) + 2]);
		}

		if(xPos < w -1 )xPos++;
		else{ xPos = 0; yPos++;}
	}
	CS_HIGH();
}
*/
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ST7735::setRotation(uint8_t m) {
	
	writecommand(ST7735_MADCTL);
	rotation = m;
	switch (rotation) {
		case 0:
		writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);

		xstart = colstart;
		ystart = rowstart;
		break;
		case 1:
		writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		
		ystart = colstart;
		xstart = rowstart;
		break;
		case 2:
		writedata(MADCTL_MH | MADCTL_BGR);
		xstart = colstart;
		ystart = rowstart-2;
		break;
		case 3:
		writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
		ystart = colstart;
		xstart = rowstart-2;
		break;
	}
}


void Adafruit_ST7735::invertDisplay(boolean i) {
	writecommand(i ? ST7735_INVON : ST7735_INVOFF);
}


/******** low level bit twiddling **********/


inline void Adafruit_ST7735::CS_HIGH(void) {
	#if defined(USE_FAST_IO)
	*csport |= cspinmask;
	#else
	//digitalWrite(_cs, HIGH);
	#endif
}

inline void Adafruit_ST7735::CS_LOW(void) {
	#if defined(USE_FAST_IO)
	*csport &= ~cspinmask;
	#else
	#error this shouldn't happen
	// digitalWrite(_cs, LOW);
	#endif
}

void Adafruit_ST7735::DC_HIGH(void) {
	
	SPCR &= ~(1 << SPE);
	//Set D/C bit to '1'
	PORTB |= (1 << PINB3);
	
	//Pulse clock line
	PORTB |= (1 << PINB5);
	
	//Reset the interface
	PORTB &= ~(1 << PINB5);
	//	PORTB &= ~(1 << PINB3);
	SPCR |= 1 <<SPE;
}

void Adafruit_ST7735::DC_LOW(void) {
	
	SPCR &= ~(1 << SPE);
	//Set D/C bit to '0'
	PORTB &= ~(1 << PINB3);

	//Pulse clock line
	PORTB |= (1 << PINB5);
	
	//Reset the interface
	PORTB &= ~(1 << PINB5);
	//PORTB &= ~(1 << PINB3);
	SPCR |= 1 <<SPE;
}
