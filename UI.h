/*
* UI.h
*
* Created: 17.4.2018 22:29:00
*  Author: Martin Hrehor
*/


#ifndef UI_H_
#define UI_H_
#include "Display/Display.h"
#include "Conf.h"

class UIElement
{
	public:
	virtual void button_pressed(uint8_t button){}
	virtual void button_held(uint8_t button, uint16_t* ctr){}
	
	UIElement(){}
};

class UIInvisibleList : public UIElement
{
	public:
	uint8_t selectedItem, itemCount;
	
	void(*onSelected)(uint8_t);
	
	UIInvisibleList(void (*_onSelected)(uint8_t), uint8_t _itemCount)
	: itemCount(_itemCount) {onSelected = _onSelected;}
	
	virtual void selectItem(uint8_t item);
	virtual void button_pressed(uint8_t button) override;
};

class UIList : public UIInvisibleList
{
	
	public:
	int16_t x,  y,  itemWidth,  itemHeight,  spacing;
	char* const* contents;
	
	UIList(void (*_onSelected)(uint8_t), int16_t _itemWidth, int16_t _itemHeight, int16_t _spacing, const char* const* _contents, uint8_t _itemCount)
	: UIInvisibleList(_onSelected, _itemCount),itemWidth(_itemWidth),itemHeight(_itemHeight),spacing(_spacing) {contents = _contents;}
	
	/*UIList(Adafruit_ST7735* _disp, void (*_onSelected)(uint8_t), int16_t _itemWidth, int16_t _itemHeight, int16_t _spacing, uint16_t _color,uint16_t _bg)
	: UIInvisibleList(_disp,_onSelected, _itemCount),itemWidth(_itemWidth),itemHeight(_itemHeight),spacing(_spacing),color(_color),bg(_bg) {onSelected = _onSelected;}*/

	void draw(int16_t _x, int16_t _y);
	void draw(int16_t _x, int16_t _y,char* _contents[], uint8_t _itemCount);
	virtual void selectItem(uint8_t item) override;
};

class UIInvisibleSlider : public UIElement
{
	public:
	int16_t minValue, maxValue, value;
	uint16_t incrementInterval;
	void(*onAccept)(int16_t);
	void(*onValueChanged)(int16_t);
	
	UIInvisibleSlider(void(*_onAccept)(int16_t),void(*_onValueChanged)(int16_t), int16_t _min,int16_t _max, uint16_t _incrementInterval, int16_t initialValue = 0)
	: minValue(_min),maxValue(_max),incrementInterval(_incrementInterval),value(initialValue){onAccept = _onAccept; onValueChanged = _onValueChanged;}
	
	virtual void button_pressed(uint8_t button) override;
	virtual void button_held(uint8_t button, uint16_t* ctr) override;
	
	virtual void update(){}
};
class UIColorSlider : public UIInvisibleSlider
{
	public:
	uint8_t x,y,w,h;
	uint16_t color;
	UIColorSlider(uint16_t _color, uint8_t _w, uint8_t _h, void(*_onAccept)(int16_t),void(*_onValueChanged)(int16_t), int16_t _min,int16_t _max, uint16_t _incrementInterval, int16_t initialValue = 0)
	: UIInvisibleSlider(_onAccept,_onValueChanged,_min,_max,_incrementInterval,initialValue), w(_w), h(_h), color(_color) {}
	
	void draw(uint8_t _x, uint8_t _y);
	virtual void update() override;
};


#endif /* UI_H_ */