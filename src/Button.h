#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
public:
    enum BUTTON_MODE
    {
        BUTTON_MODE_PULLUP,
        BUTTON_MODE_PULLDOWN,
        BUTTON_MODE_EXTERNAL
    };

    enum BUTTON_STATE
    {
        BUTTON_STATE_NONE,
        BUTTON_STATE_PRESSED,
        BUTTON_STATE_RELEASED,
        BUTTON_STATE_SHORT_PRESS,
        BUTTON_STATE_LONG_PRESS
    };

    enum BUTTON_INTERNAL_STATE
    {
        BUTTON_INTERNAL_IDLE,
        BUTTON_INTERNAL_PRESSED,
        BUTTON_INTERNAL_LONG
    };
    
    Button::BUTTON_STATE state;

    Button(uint8_t pin,
           Button::BUTTON_MODE mode = Button::BUTTON_MODE_PULLUP,
           uint16_t debounceMs = 40,
           uint16_t longPressMs = 800);

    void modeInit();
    void update();

    bool isPressed();
    bool isReleased();

private:
    uint8_t _pin;
    Button::BUTTON_MODE _mode;
    BUTTON_INTERNAL_STATE _internalState;

    uint16_t _debounceMs;
    uint16_t _longPressMs;

    bool _lastRead;
    bool _currentRead;

    unsigned long _lastDebounceTime;
    unsigned long _pressStartTime;

    bool _longFired;

    bool activeLevel(); // LOW ou HIGH selon mode
};

#endif