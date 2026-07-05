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
#define MAIN_2


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

#define BUTTON_PIN 22

Button btn(BUTTON_PIN, Button::BUTTON_MODE_PULLUP, 40, 800);


void setup()
{
    Serial.begin(115200);
    Serial.printf("Setup done !");
}

void loop()
{
    btn.update();
    // Serial.printf("%u\n", btn.state);
    // delay(10);
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