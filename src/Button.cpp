#include "Button.h"

Button::Button(uint8_t pin, Button::BUTTON_MODE mode, uint16_t debounceMs, uint16_t longPressMs)
{
    _pin  = pin;
    _mode = mode;

    _debounceMs  = debounceMs;
    _longPressMs = longPressMs;

    _lastRead    = !activeLevel();
    _currentRead = !activeLevel();

    _lastDebounceTime = 0;
    _pressStartTime   = 0;

    _longFired = false;

    _internalState = BUTTON_INTERNAL_IDLE;
    state = BUTTON_STATE_NONE;

    modeInit();
}

void Button::modeInit()
{
    switch (_mode)
    {
        case BUTTON_MODE_PULLUP:
            pinMode(_pin, INPUT_PULLUP);
            break;

        case BUTTON_MODE_PULLDOWN:
            pinMode(_pin, INPUT_PULLDOWN);
            break;

        case BUTTON_MODE_EXTERNAL:
            pinMode(_pin, INPUT);
            break;
    }
}

bool Button::activeLevel()
{
    return _mode == PULLDOWN ? HIGH : LOW;
}

// bool Button::isPressed()
// {
//     return state == activeLevel();
// }

// bool Button::isReleased()
// {
//     return state != activeLevel();
// }

void Button::update()
{
    state = BUTTON_STATE_NONE;

    bool currentRead = digitalRead(_pin);

    // Debounce
    if (currentRead != _lastRead)
    {
        _lastDebounceTime = millis();
        _lastRead = currentRead;
    }

    if (millis() - _lastDebounceTime < _debounceMs)
        return;

    // ==========================
    // Bouton appuyé
    // ==========================
    if (currentRead == activeLevel())
    {
        switch (_internalState)
        {
            case BUTTON_INTERNAL_IDLE:

                _internalState = BUTTON_INTERNAL_PRESSED;
                _pressStartTime = millis();
                _longFired = false;

                state = BUTTON_STATE_PRESSED;

                Serial.printf("[BTN %u] PRESSED callback\n", _pin);
                break;

            case BUTTON_INTERNAL_PRESSED:

                if (!_longFired &&
                    millis() - _pressStartTime >= _longPressMs)
                {
                    _longFired = true;
                    _internalState = BUTTON_INTERNAL_LONG;

                    state = BUTTON_STATE_LONG_PRESS;

                    Serial.printf("[BTN %u] LONG_PRESS callback\n", _pin);
                }

                break;

            case BUTTON_INTERNAL_LONG:
                break;
        }
    }

    // ==========================
    // Bouton relâché
    // ==========================
    else
    {
        switch (_internalState)
        {
            case BUTTON_INTERNAL_IDLE:
                break;

            case BUTTON_INTERNAL_PRESSED:

                state = BUTTON_STATE_SHORT_PRESS;

                Serial.printf("[BTN %u] SHORT_PRESS callback\n", _pin);

                _internalState = BUTTON_INTERNAL_IDLE;
                break;

            case BUTTON_INTERNAL_LONG:

                state = BUTTON_STATE_RELEASED;

                Serial.printf("[BTN %u] RELEASED callback\n", _pin);

                _internalState = BUTTON_INTERNAL_IDLE;
                break;
        }
    }
}