#pragma once
/*
 * Button.h
 * --------
 * Lib ESP32 (Arduino core) pour gérer un bouton poussoir avec :
 *   - Détection du front (appui/relâchement) via interruption GPIO (attachInterruptArg)
 *   - Anti-rebond (debounce) et distinction appui court / appui long via
 *     une machine à états rejouée automatiquement par un timer matériel (esp_timer)
 *   - Callbacks appelés en contexte "tâche timer" (donc en dehors de l'ISR brute),
 *     ce qui permet d'y faire du Serial.print, allouer de la mémoire, etc. sans risque.
 *
 * Aucune action bloquante n'est faite dans l'ISR : elle se contente de lever un
 * flag volatile. Tout le traitement (debounce + timing) est fait par le timer
 * périodique interne, qui appelle update().
 */

#include <Arduino.h>
#include <functional>
#include "esp_timer.h"

class Button {
public:
    using Callback = std::function<void()>;

    // pin        : broche GPIO du bouton
    // activeLow  : true si le bouton relie la broche à la masse (GND) quand pressé (cas le plus courant)
    // usePullup  : true pour activer la résistance de pull interne (PULLUP si activeLow, PULLDOWN sinon)
    Button(uint8_t pin, bool activeLow = true, bool usePullup = true);
    ~Button();

    // Configure la broche, l'interruption GPIO et démarre le timer interne de scrutation.
    void begin();

    // Désactive l'interruption et arrête le timer interne (libère les ressources).
    void end();

    // --- Callbacks ---
    void onPress(Callback cb);       // Appui court validé : bouton relâché avant le seuil "long press"
    void onLongPress(Callback cb);   // Déclenché UNE fois, dès que le seuil long est atteint (bouton encore maintenu)
    void onRelease(Callback cb);     // Relâchement du bouton, que l'appui ait été court ou long

    // --- Réglages ---
    void setDebounceTime(uint16_t ms);     // Défaut : 30 ms
    void setLongPressTime(uint16_t ms);    // Défaut : 800 ms
    void setScanInterval(uint16_t ms);     // Défaut : 5 ms (période du timer interne, à régler avant begin())

    bool isPressed() const;
    uint8_t getPin() const { return _pin; }

    // Appelée automatiquement par le timer interne à chaque tick.
    // Peut aussi être appelée manuellement si vous préférez tout gérer depuis loop()
    // (dans ce cas, appelez end() avant, ou ne démarrez pas le timer).
    void update();

private:
    enum class State : uint8_t {
        IDLE,
        DEBOUNCE_PRESS,
        PRESSED,
        LONG_PRESSED,
        DEBOUNCE_RELEASE
    };

    static void IRAM_ATTR gpioIsr(void* arg);
    static void timerCallback(void* arg);

    bool readPressed() const;

    uint8_t  _pin;
    bool     _activeLow;
    bool     _usePullup;

    volatile bool _gpioChanged; // Simple indicateur (utile pour debug/stat), pas indispensable au fonctionnement

    uint16_t _debounceMs;
    uint16_t _longPressMs;
    uint16_t _scanIntervalMs;

    State    _state;
    uint32_t _stateEnterTime;
    bool     _longFired;

    Callback _cbPress;
    Callback _cbLongPress;
    Callback _cbRelease;

    esp_timer_handle_t _timerHandle;
    bool _started;
};
