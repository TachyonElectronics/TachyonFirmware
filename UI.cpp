/*
* UI.cpp
*
* Created: 17.4.2018 22:29:15
*  Author: DELTA-PC
*/

#include "UI.h"

void UIInvisibleList::selectItem(uint8_t item)
{
	selectedItem = item;
	//onSelected(item);
}
void UIList::selectItem(uint8_t item)
{
	disp->fillRect(x+1,y+1+(itemHeight+spacing)*selectedItem,itemWidth-2,itemHeight-2,*bg);
	disp->setTextColor(*color,*bg);
	disp->setCursor(x+2,y+2+(itemHeight+spacing)*selectedItem);
	disp->print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&contents[selectedItem]));
	
	selectedItem = item;
	
	disp->fillRect(x,y+(itemHeight+spacing)*selectedItem,itemWidth,itemHeight,*color);
	disp->setTextColor(*bg,*color);
	disp->setCursor(x+2,y+2+(itemHeight+spacing)*selectedItem);
	disp->print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&contents[selectedItem]));
}

void UIList::draw(int16_t _x, int16_t _y)
{
	x = _x;
	y = _y;
	
	disp->setFont();
	disp->setTextSize(1);
	for (uint8_t i = 0; i < itemCount; i++)
	{
		if(i == selectedItem)
		{
			disp->fillRect(x,y+(itemHeight+spacing)*i,itemWidth,itemHeight,*color);
			disp->setTextColor(*bg,*color);
		}
		else
		{
			disp->drawRect(x,y+(itemHeight+spacing)*i,itemWidth,itemHeight,*color);
			disp->setTextColor(*color,*bg);
		}
		disp->setCursor(x+2,y+2+(itemHeight+spacing)*i);
		disp->print(reinterpret_cast<const __FlashStringHelper *> pgm_read_word(&contents[i]));
	}
}


void UIList::draw(int16_t _x, int16_t _y,char* _contents[], uint8_t _itemCount)
{
	x = _x;
	y = _y;
	
	itemCount = _itemCount;

	disp->setFont();
	disp->setTextSize(1);

	for (uint8_t i = 0; i < itemCount; i++)
	{
		if(i == selectedItem)
		{
			disp->fillRect(x,y+(itemHeight+spacing)*i,itemWidth,itemHeight,*color);
			disp->setTextColor(*bg,*color);
		}
		else
		{
			disp->drawRect(x,y+(itemHeight+spacing)*i,itemWidth,itemHeight,*color);
			disp->setTextColor(*color,*bg);
		}
		disp->setCursor(x+2,y+2+(itemHeight+spacing)*i);
		disp->print(_contents[i]);
	}
}




void UIInvisibleList::button_pressed(uint8_t button)
{
	switch(button)
	{
		case BTN_CENTER:
		onSelected(selectedItem);
		return;
		
		case BTN_RIGHT:
		if(selectedItem)//don't go up when already at zero!
		selectItem(selectedItem-1);
		return;
		
		case BTN_LEFT:
		if(selectedItem < itemCount - 1) //same here, don't go down when at last item
		selectItem(selectedItem+1);
		return;
	}
}


void UIInvisibleSlider::button_pressed(uint8_t button)
{
	if(button == BTN_CENTER) onAccept(value);
}

void UIInvisibleSlider::button_held(uint8_t button, uint16_t* ctr)
{
	if(*ctr > incrementInterval)
	{
		switch(button){
			default:
			return;
			
			case BTN_UP:
			value = value < maxValue? value + 1: maxValue;
			break;
			
			case BTN_DOWN:
			value = value > minValue? value - 1: minValue;
			break;
		}
		update();
		onValueChanged(value);
		*ctr = 0;
	}
}

void UIVerticalSlider::draw(uint8_t _x, uint8_t _y)
{
	x = _x; y = _y;
	disp->drawRect(x,y,w,h,*uiColor); //Draw outline
	update();
}

void UIVerticalSlider::update()
{
	uint8_t crop = map(value,minValue,maxValue,h-2,0);
	disp->fillCroppedRect(x+1,y+1,w-2,h-2,*uiColor,*bgColor, 0,-crop);
}

