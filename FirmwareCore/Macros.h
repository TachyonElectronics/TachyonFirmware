#ifndef MACROS_H_
#define MACROS_H_

//Macros
extern uint16_t buttonCtrs[_BTN_COUNT];
#define DECLARE_BUTTON(name) extern bool name##_isPressed; extern uint16_t _##name##_debounceCtr;
#define DEFINE_BUTTON(name) bool name##_isPressed = 0; uint16_t _##name##_debounceCtr = 0;

#define HANDLE_BUTTON_INTERRUPT(name, port, pin) if(bit_is_clear(port,pin)) { if(!name##_isPressed && _##name##_debounceCtr >= BUTTON_DEBOUNCE_INTERVAL) {buttonPressed(name);name##_isPressed=1;buttonCtrs[name]=0; _##name##_debounceCtr = 0; } } else if(name##_isPressed){name##_isPressed = 0; _##name##_debounceCtr = 0;}
#define HANDLE_BUTTON_LOOP(name) if(name##_isPressed){buttonCtrs[name] += deltaMillis; buttonHeld(name);}  if(_##name##_debounceCtr < BUTTON_DEBOUNCE_INTERVAL)_##name##_debounceCtr += deltaMillis;
	/*
#define openSimpleListScreen(_list, title, _screen) currentScreen = _screen; disp.fillScreen(settings.bgColor); disp.setTextColor(settings.uiColor); disp.setCursor(40,0); disp.print(title); _list.draw(8,12); focusedUiElement = &_list;
#define openSimpleListScreen_xy(_list, title, _screen,_x,_y) currentScreen = _screen; disp.fillScreen(settings.bgColor); disp.setTextColor(settings.uiColor); disp.setCursor(40,0); disp.print(title); _list.draw(_x,_y); focusedUiElement = &_list;
*/


#define SHIFT_RED 11
#define SHIFT_GREEN 5
#define SHIFT_BLUE 0
#define GET_RED(color) ((color & 0xF800) >> 11)
#define GET_GREEN(color) ((color & 0x7E0) >> 5)
#define GET_BLUE(color) (color & 0x1F)
#define PROGMEMSTRING(s) ((__FlashStringHelper*)(s))



#endif /* MACROS_H_ */