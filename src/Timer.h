/*
 * Timer.h
 *
 * Timer principal ESP32, sur le meme principe qu'un SysTick_Handler
 * STM32 : un tick de base (1ms) qui descend en cascade vers des
 * callbacks 10ms / 100ms / 1000ms.
 *
 * Usage :
 *
 *   #include "Timer.h"
 *
 *   void setup() {
 *       timerInit();
 *   }
 *
 * Pas besoin d'appeler quoi que ce soit dans loop() : les callbacks
 * sont appelees directement par le timer materiel (voir Timer.cpp).
 *
 * Implemente timerCallback10ms(), timerCallback100ms() et
 * timerCallback1000ms() quelque part dans ton projet (ex: dans ton
 * .ino, ou dans un autre .cpp) pour y mettre tes fonctions recurrentes.
 * Laisse le corps vide si tu n'as besoin que d'un seul niveau.
 */

#pragma once

#include <Arduino.h>
#include "esp_timer.h"

// ---- Inclue ici les librairies dont tes fonctions recurrentes ont besoin ----
// #include "MaLib.h"


// A implementer par toi-meme (ex: dans ton .ino)
void timerCallback10ms(void);
void timerCallback100ms(void);
void timerCallback1000ms(void);

// Demarre le timer materiel (tick de base = 1ms).
void timerInit(void);

// Arrete le timer materiel.
void timerDeinit(void);
