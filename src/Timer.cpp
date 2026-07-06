#include "Timer.h"

static esp_timer_handle_t timerHandle = nullptr;

// Appelee toutes les 1ms par le timer materiel (esp_timer, dispatch en
// tache, donc pas de restriction stricte d'ISR : les appels ci-dessous
// sont sans risque, comme s'ils venaient de la boucle principale).
static void onTimerTick(void* arg) {
#warning "All timer interrupts are roughly precise"
    static uint32_t timer_10ms   = 9; // 9 => 10 pas (9..0) = 10ms
    static uint32_t timer_100ms  = 9;
    static uint32_t timer_1000ms = 9;

    if (timer_10ms > 0) {
        timer_10ms--;
    } else {
        timer_10ms = 9;
        timerCallback10ms();

        if (timer_100ms > 0) {
            timer_100ms--;
        } else {
            timer_100ms = 9;
            timerCallback100ms();

            if (timer_1000ms > 0) {
                timer_1000ms--;
            } else {
                timer_1000ms = 9;
                timerCallback1000ms();
            }
        }
    }
}

void timerInit(void) {
    esp_timer_create_args_t args = {};
    args.callback = &onTimerTick;
    args.arg = nullptr;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "main_timer";

    esp_timer_create(&args, &timerHandle);
    esp_timer_start_periodic(timerHandle, 10*1000ULL); // 1000us = 1ms
}

void timerDeinit(void) {
    if (timerHandle) {
        esp_timer_stop(timerHandle);
        esp_timer_delete(timerHandle);
        timerHandle = nullptr;
    }
}

void timerCallback10ms(void)
{

}

void timerCallback100ms(void)
{

}

void timerCallback1000ms(void)
{

}
