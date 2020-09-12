/*
* RTC.h
*
* Created: 7.5.2018 21:21:17
*  Author: Martin Hrehor
*/


#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

#define _RTC_ADDRESS 0b1101111
#define RTC_MINUTES 0x01
#define RTC_HOURS 0x02
#define RTC_CALIBRATION 0x08

class RTC
{
	
	public:
	uint16_t seconds, minutes, hours, lastResult;
	
	void Write(uint8_t address, uint8_t data);
	uint8_t Read(uint8_t addresss);
	
	void Update();
	void SetTime();
	// Convert Decimal to Binary Coded Decimal (BCD)
	uint8_t dec2bcd(uint8_t num)
	{
		return ((num/10 * 16) + (num % 10));
	}

	// Convert Binary Coded Decimal (BCD) to Decimal
	uint8_t bcd2dec(uint8_t num)
	{
		return ((num/16 * 10) + (num % 16));
	}
	
	void Init();

};

#endif /* RTC_H_ */