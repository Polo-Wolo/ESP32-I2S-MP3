#include "Button.h"

// ---- Membres statiques (partages entre toutes les instances) ----
Button* Button::_instances[BUTTON_MAX_INSTANCES] = { nullptr };
uint8_t Button::_instanceCount = 0;
esp_timer_handle_t Button::_sharedTimerHandle = nullptr;
uint16_t Button::_scanIntervalMs = 5;

Button::Button(uint8_t pin, bool activeLow, bool usePullup)
    : _pin(pin),
      _activeLow(activeLow),
      _usePullup(usePullup),
      _gpioChanged(false),
      _debounceMs(30),
      _longPressMs(700),
      _state(State::IDLE),
      _stateEnterTime(0),
      _longFired(false),
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

void Button::setScanInterval(uint16_t ms) {
    _scanIntervalMs = ms; // pris en compte au prochain (re)demarrage du timer partage
}

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
    // Contexte de la tache esp_timer (pas l'ISR GPIO) : sans risque d'y
    // parcourir tous les boutons et d'appeler leurs callbacks utilisateur.
    updateAll();
}

void Button::updateAll() {
    for (uint8_t i = 0; i < BUTTON_MAX_INSTANCES; i++) {
        if (_instances[i] != nullptr) {
            _instances[i]->update();
        }
    }
}

bool Button::registerInstance(Button* btn) {
    for (uint8_t i = 0; i < BUTTON_MAX_INSTANCES; i++) {
        if (_instances[i] == btn) return true; // deja enregistre
    }
    for (uint8_t i = 0; i < BUTTON_MAX_INSTANCES; i++) {
        if (_instances[i] == nullptr) {
            _instances[i] = btn;
            _instanceCount++;
            return true;
        }
    }
    return false; // plus de place (augmente BUTTON_MAX_INSTANCES)
}

void Button::unregisterInstance(Button* btn) {
    for (uint8_t i = 0; i < BUTTON_MAX_INSTANCES; i++) {
        if (_instances[i] == btn) {
            _instances[i] = nullptr;
            _instanceCount--;
            break;
        }
    }
}

void Button::ensureSharedTimerStarted() {
    if (_sharedTimerHandle != nullptr) return; // deja demarre

    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = &Button::timerCallback;
    timerArgs.arg = nullptr;
    timerArgs.dispatch_method = ESP_TIMER_TASK;
    timerArgs.name = "button_scan";

    esp_timer_create(&timerArgs, &_sharedTimerHandle);
    esp_timer_start_periodic(_sharedTimerHandle, (uint64_t)_scanIntervalMs * 1000ULL);
}

void Button::stopSharedTimerIfUnused() {
    if (_instanceCount == 0 && _sharedTimerHandle != nullptr) {
        esp_timer_stop(_sharedTimerHandle);
        esp_timer_delete(_sharedTimerHandle);
        _sharedTimerHandle = nullptr;
    }
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

    // Interruption GPIO individuelle : detecte les fronts montants ET descendants.
    attachInterruptArg(digitalPinToInterrupt(_pin), &Button::gpioIsr, this, CHANGE);

    // Enregistrement dans la liste partagee, puis demarrage (une seule fois)
    // du timer commun a tous les boutons.
    registerInstance(this);
    ensureSharedTimerStarted();

    _started = true;
}

void Button::end() {
    if (_started) {
        detachInterrupt(digitalPinToInterrupt(_pin));
        unregisterInstance(this);
        stopSharedTimerIfUnused();
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
                // Rebond : on annule et on revient a IDLE
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
                // Rebond au relachement : on revient a l'etat precedent
                _state = _longFired ? State::LONG_PRESSED : State::PRESSED;
            } else if (now - _stateEnterTime >= _debounceMs) {
                _state = State::IDLE;
                if (_cbRelease) _cbRelease();
                if (!_longFired && _cbPress) _cbPress(); // appui court confirme au relachement
            }
            break;
    }
}