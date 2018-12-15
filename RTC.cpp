/*
* RTC.cpp
*
* Created: 7.5.2018 21:01:38
*  Author: Martin Hrehor
*/

#include "RTC.h"
#include <Wire.h>

void RTC::Init()
{
	uint8_t reg = Read(0x00);
	if(!(reg & 0x80))
	Write(0,0x80); //Ensure oscillator is startred
	
	reg = Read(0x02);
	if(reg & 0x40)
	Write(0x02, reg & ~(0x40)); //Ensure 24hr format
	
	reg = Read(0x03);
	Write(0x03,(0x08 | reg)); //Ensure external backup is enabled and device is set to normal power
	
}

uint8_t RTC::Read(uint8_t address)
{
	Wire.beginTransmission(_RTC_ADDRESS);
	Wire.write(address);
	lastResult = Wire.endTransmission();
	Wire.requestFrom(_RTC_ADDRESS, 1);
	uint8_t recvd = Wire.read();
	return recvd;
}

void RTC::Write(uint8_t address, uint8_t data)
{
	Wire.beginTransmission(_RTC_ADDRESS);
	Wire.write(address);
	Wire.write(data);
	Wire.endTransmission();
}

void RTC::Update()
{
	Wire.beginTransmission(_RTC_ADDRESS);
	Wire.write(0);
	Wire.endTransmission();
	Wire.requestFrom(_RTC_ADDRESS, 3);
	seconds = bcd2dec(Wire.read() & 0x7F);
	minutes = bcd2dec(Wire.read());
	hours = bcd2dec(Wire.read() & 0x3F);
}
void RTC::SetTime()
{
	Wire.beginTransmission(_RTC_ADDRESS);
	Wire.write(0);
	Wire.write(0x80);
	Wire.write(dec2bcd(minutes));
	Wire.write(dec2bcd(hours));
	Wire.endTransmission();
}