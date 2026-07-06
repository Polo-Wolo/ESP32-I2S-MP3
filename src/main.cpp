//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32,                                                              *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 4  , Aug.22/2025
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//
#define MAIN_4


#ifdef MAIN_1
#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

// void my_audio_info(Audio::msg_t m) {
//     Serial.printf("%s: %s\n", m.s, m.msg);
// }

void setup() {
    // Audio::audio_info_callback = my_audio_info;
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    Serial.begin(115200);
    SD.begin(SD_CS);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0...21
    audio.connecttoFS(SD, "Test.mp3");
}

void loop(){
    vTaskDelay(1);
    audio.loop();
}
#endif
#ifdef MAIN_2

#include <Arduino.h>

#include "Button.h"
#include "config.h"

// #define BUTTON_PIN 22


// Button btn(BUTTON_PIN, Button::BUTTON_MODE_PULLUP);


#include "Button.h"

// Bouton câblé entre GPIO4 et GND (pull-up interne activée)
Button btn1(22, true, true);

// Bouton câblé entre GPIO5 et GND
Button btn2(34, true, true);

void toto() {
    ets_printf("%u [BTN 1] Long press from toto\n", millis());
}

void setup() {
    Serial.begin(115200);

    btn1.setDebounceTime(30);     // ms
    btn1.setLongPressTime(800);   // ms

    btn1.onPress([]() {
        ets_printf("%u [BTN 1] Short press\n", millis());
    });
    btn1.onLongPress(toto);

    btn1.onRelease([]() {
        ets_printf("%u [BTN 1] Release\n", millis());
    });

    btn2.onPress([]() {
        ets_printf("%u [BTN 2] Short press\n", millis());
    });
    btn2.onLongPress([]() {
        ets_printf("%u [BTN 2] Long press\n", millis());
    });
    btn2.onRelease([]() {
        ets_printf("%u [BTN 2] Release\n", millis());
    });

    btn1.begin();
    btn2.begin();
}

void loop() {
    // Rien à faire ici : la détection tourne en tâche de fond
    // (interruption GPIO + timer interne). Vous pouvez laisser
    // loop() libre pour le reste de votre logique.
    delay(10);
}


#endif
#ifdef MAIN_3

#include <Arduino.h>

#define PIN 22

uint8_t counter = 0;

void IRAM_ATTR changed()
{
    if (digitalRead(PIN)) ets_printf("released\n");
    else ets_printf("pressed\n");
}

void setup()
{
    Serial.begin(115200);
    pinMode(PIN, INPUT_PULLUP);
    attachInterrupt(PIN, changed, CHANGE);
    Serial.printf("Setup DONE\n");
}

void loop()
{
    
}

#endif

#ifdef MAIN_4

#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

// Personnal libs
#include "Button.h"
#include "config.h"

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

// Bouton câblé entre GPIO4 et GND (pull-up interne activée)
Button btn1(22, true, true);

uint8_t volume = 10;

void increaseVolume() {
    if (volume < 21) {
        volume++;
        audio.setVolume(volume);
        ets_printf("%u [BTN 1] Volume increased to %d\n", millis(), volume);
    }
}

void decreaseVolume() {
    if (volume > 0) {
        volume--;
        audio.setVolume(volume);
        ets_printf("%u [BTN 1] Volume decreased to %d\n", millis(), volume);
    }
}

void setup() {

    btn1.setDebounceTime(30);     // ms
    btn1.setLongPressTime(800);   // ms

    btn1.onPress(increaseVolume);
    btn1.onLongPress(decreaseVolume);
    btn1.onRelease([]() { ets_printf("%u [BTN 1] Release\n", millis()); });

    btn1.begin();

    // Audio::audio_info_callback = my_audio_info;
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    Serial.begin(115200);
    SD.begin(SD_CS);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0...21
    audio.connecttoFS(SD, "Test.mp3");
}

void loop(){
    vTaskDelay(1);
    audio.loop();
}

#endif