/*
 * Button.h
 *
 * Gestion d'un bouton avec debounce + detection d'appui long, sur ESP32.
 *
 * Un seul timer materiel (esp_timer) est partage par TOUTES les
 * instances de Button : chaque bouton s'enregistre au begin() dans
 * une liste statique, et le timer commun appelle update() sur chacun
 * d'eux a intervalle regulier (scan interval global, 5ms par defaut).
 *
 * L'interruption GPIO (une par pin) reste individuelle : elle ne sert
 * qu'a lever un flag pour reveiller/valider l'etat plus vite si besoin,
 * le vrai traitement (debounce, callbacks) se fait dans update().
 */

#pragma once

#include <Arduino.h>
#include "esp_timer.h"
#include <functional>

#ifndef BUTTON_MAX_INSTANCES
#define BUTTON_MAX_INSTANCES 16
#endif

class Button {
public:
    using Callback = std::function<void(void)>;

    Button(uint8_t pin, bool activeLow = true, bool usePullup = true);
    ~Button();

    void begin();
    void end();

    void onPress(Callback cb);
    void onLongPress(Callback cb);
    void onRelease(Callback cb);

    void setDebounceTime(uint16_t ms);
    void setLongPressTime(uint16_t ms);

    bool isPressed() const;

    // Intervalle de scan GLOBAL, partage par tous les boutons.
    // A appeler avant le premier begin() pour avoir effet (le timer
    // partage est cree au tout premier begin()).
    static void setScanInterval(uint16_t ms);

private:
    bool readPressed() const;
    void update(); // machine a etats, appelee par le timer partage

    static void IRAM_ATTR gpioIsr(void* arg);
    static void timerCallback(void* arg); // callback du timer partage
    static void updateAll();              // appelle update() sur tous les boutons enregistres

    static bool registerInstance(Button* btn);
    static void unregisterInstance(Button* btn);
    static void ensureSharedTimerStarted();
    static void stopSharedTimerIfUnused();

    enum class State {
        IDLE,
        DEBOUNCE_PRESS,
        PRESSED,
        LONG_PRESSED,
        DEBOUNCE_RELEASE
    };

    uint8_t _pin;
    bool _activeLow;
    bool _usePullup;

    volatile bool _gpioChanged;

    uint16_t _debounceMs;
    uint16_t _longPressMs;

    State _state;
    uint32_t _stateEnterTime;
    bool _longFired;

    bool _started;

    Callback _cbPress;
    Callback _cbLongPress;
    Callback _cbRelease;

    // ---- Partage entre toutes les instances ----
    static Button* _instances[BUTTON_MAX_INSTANCES];
    static uint8_t _instanceCount;

    static esp_timer_handle_t _sharedTimerHandle;
    static uint16_t _scanIntervalMs;
};