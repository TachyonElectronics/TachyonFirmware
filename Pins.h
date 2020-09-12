/*
 * Pins.h
 *
 * Created: 15.12.2018 12:30:28
 *  Author: DELTA-PC
 */ 


#ifndef PINS_H_
#define PINS_H_

/*====================================================
							V1
======================================================*/

#if defined(ARDUINO_TACHYON_V1)

#define PINREGISTER_BTN_CENTER PINC
#define PIN_BTN_CENTER PINC1

#define PINREGISTER_BTN_LEFT PINC
#define PIN_BTN_LEFT PINC0

#define PINREGISTER_BTN_RIGHT PINC
#define PIN_BTN_RIGHT PINC2

#define BTN_UP BTN_RIGHT //These are just aliases
#define BTN_DOWN BTN_LEFT

#define RELOAD_INTERRUPT 0
#define RELOAD_PINREGISTER PIND
#define RELOAD_PORT PORTD
#define RELOAD_PIN PIND2

#define BATTERY_ADC_CHANNEL 6

#define SENSOR_INTERRUPT 1
#define SENSOR_PINREGISTER PIND
#define SENSOR_PORT PORTD
#define SENSOR_PIN PIND3

#define DSP_BL 5
#define DSP_CS 8
#define DSP_RST 10

/*====================================================
							V2
======================================================*/


#elif defined(ARDUINO_TACHYON_V2)

#define PINREGISTER_BTN_CENTER PINC
#define PIN_BTN_CENTER PINC1

#define PINREGISTER_BTN_LEFT PINC
#define PIN_BTN_LEFT PINC2

#define PINREGISTER_BTN_RIGHT PINC
#define PIN_BTN_RIGHT PINC0

#define BTN_UP BTN_RIGHT //These are just aliases
#define BTN_DOWN BTN_LEFT

#define RELOAD_INTERRUPT 0
#define RELOAD_PINREGISTER PIND
#define RELOAD_PORT PORTD
#define RELOAD_PIN PIND2

#define BATTERY_ADC_CHANNEL 3

#define SENSOR_INTERRUPT 1
#define SENSOR_PINREGISTER PIND
#define SENSOR_PORT PORTD
#define SENSOR_PIN PIND3

#define DSP_BL 5
#define DSP_CS 8
#define DSP_RST 10

/*====================================================
							V3
======================================================*/

//TODO:: 
//#elif defined(ARDUINO_TACHYON_V2)
#else

#define PINREGISTER_BTN_CENTER PINC
#define PIN_BTN_CENTER PINC0

#define PINREGISTER_BTN_LEFT PINE
#define PIN_BTN_LEFT PINE3

#define PINREGISTER_BTN_RIGHT PINC
#define PIN_BTN_RIGHT PINC1

#define BTN_UP BTN_RIGHT //These are just aliases
#define BTN_DOWN BTN_LEFT

#define RELOAD_INTERRUPT 0
#define RELOAD_PINREGISTER PIND

#define RELOAD_PIN PIND2
#define RELOAD_PORT PORTD
#define BATTERY_ADC_CHANNEL 3

#define SENSOR_INTERRUPT 1
#define SENSOR_PINREGISTER PIND
#define SENSOR_PORT PORTD
#define SENSOR_PIN PIND3

#define DSP_BL 5
#define DSP_CS 8
#define DSP_RST 10

#define SPCR SPCR0
#define SPSR SPSR0

#define HAS_MAGNETOMETER

//TODO: uncomment
//Throw a compilation error if target board is not a Tachyon
//#else
//#error Build target not supported. Please select a compatible Tachyon board
#endif

#endif /* PINS_H_ */