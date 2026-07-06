#include "Button.h"

Button::Button(uint8_t pin, bool activeLow, bool usePullup)
    : _pin(pin),
      _activeLow(activeLow),
      _usePullup(usePullup),
      _gpioChanged(false),
      _debounceMs(30),
      _longPressMs(2000),
      _scanIntervalMs(5),
      _state(State::IDLE),
      _stateEnterTime(0),
      _longFired(false),
      _timerHandle(nullptr),
      _started(false)
{
}

Button::~Button() {
    end();
}

void Button::onPress(Callback cb)     { _cbPress = cb; }
void Button::onLongPress(Callback cb) { _cbLongPress = cb; }
void Button::onRelease(Callback cb)   { _cbRelease = cb; }

void Button::setDebounceTime(uint16_t ms)  { _debounceMs = ms; }
void Button::setLongPressTime(uint16_t ms) { _longPressMs = ms; }
void Button::setScanInterval(uint16_t ms)  { _scanIntervalMs = ms; }

bool Button::readPressed() const {
    bool raw = digitalRead(_pin);
    return _activeLow ? (raw == LOW) : (raw == HIGH);
}

bool Button::isPressed() const {
    return (_state == State::PRESSED) || (_state == State::LONG_PRESSED);
}

void IRAM_ATTR Button::gpioIsr(void* arg) {
    // ISR minimaliste : on ne fait QUE lever un flag, aucun traitement lourd ici.
    Button* self = static_cast<Button*>(arg);
    self->_gpioChanged = true;
}

void Button::timerCallback(void* arg) {
    // Ce callback s'exécute dans le contexte de la tâche esp_timer (pas dans l'ISR GPIO),
    // il est donc sûr d'y appeler les callbacks utilisateur (std::function, Serial, etc.).
    Button* self = static_cast<Button*>(arg);
    self->update();
}

void Button::begin() {
    if (_usePullup) {
        pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
    } else {
        pinMode(_pin, INPUT);
    }

    _state = State::IDLE;
    _longFired = false;
    _stateEnterTime = millis();

    // Interruption GPIO : détecte les fronts montants ET descendants.
    attachInterruptArg(digitalPinToInterrupt(_pin), &Button::gpioIsr, this, CHANGE);

    // Timer périodique interne : rejoue la machine à états (debounce + long press)
    // sans que l'utilisateur ait besoin d'appeler update() dans loop().
    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = &Button::timerCallback;
    timerArgs.arg = this;
    timerArgs.dispatch_method = ESP_TIMER_TASK;
    timerArgs.name = "button_scan";

    esp_timer_create(&timerArgs, &_timerHandle);
    esp_timer_start_periodic(_timerHandle, (uint64_t)_scanIntervalMs * 1000ULL);
    _started = true;
}

void Button::end() {
    if (_started) {
        detachInterrupt(digitalPinToInterrupt(_pin));
        if (_timerHandle) {
            esp_timer_stop(_timerHandle);
            esp_timer_delete(_timerHandle);
            _timerHandle = nullptr;
        }
        _started = false;
    }
}

void Button::update() {
    bool pressed = readPressed();
    uint32_t now = millis();

    switch (_state) {

        case State::IDLE:
            if (pressed) {
                _state = State::DEBOUNCE_PRESS;
                _stateEnterTime = now;
            }
            break;

        case State::DEBOUNCE_PRESS:
            if (!pressed) {
                // Rebond : on annule et on revient à IDLE
                _state = State::IDLE;
            } else if (now - _stateEnterTime >= _debounceMs) {
                _state = State::PRESSED;
                _stateEnterTime = now;
                _longFired = false;
            }
            break;

        case State::PRESSED:
            if (!pressed) {
                _state = State::DEBOUNCE_RELEASE;
                _stateEnterTime = now;
            } else if (!_longFired && (now - _stateEnterTime >= _longPressMs)) {
                _longFired = true;
                _state = State::LONG_PRESSED;
                if (_cbLongPress) _cbLongPress();
            }
            break;

        case State::LONG_PRESSED:
            if (!pressed) {
                _state = State::DEBOUNCE_RELEASE;
                _stateEnterTime = now;
            }
            break;

        case State::DEBOUNCE_RELEASE:
            if (pressed) {
                // Rebond au relâchement : on revient à l'état précédent
                _state = _longFired ? State::LONG_PRESSED : State::PRESSED;
            } else if (now - _stateEnterTime >= _debounceMs) {
                _state = State::IDLE;
                if (_cbRelease) _cbRelease();
                if (!_longFired && _cbPress) _cbPress(); // appui court confirmé au relâchement
            }
            break;
    }
}
