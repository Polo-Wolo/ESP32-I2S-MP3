#pragma once
/*
 * SDBrowser.h
 * -----------
 * Explorateur de fichiers pour carte SD (bus SPI) sur ESP32.
 * Gère uniquement l'état de navigation (dossier courant, sélection,
 * entrer/sortir d'un dossier, affichage) — indépendant de la source des
 * événements de navigation (boutons physiques, série, etc.).
 *
 * Affichage via ets_printf() : fonction ROM bas niveau qui écrit
 * directement sur l'UART0, sans passer par le buffer logiciel de Serial.
 * Limite : ets_printf ne supporte pas %f (pas de flottants).
 *
 * Dépendance optionnelle : ButtonLib, uniquement pour la méthode
 * attachButtons() qui est un raccourci pratique. Le reste de la lib
 * ne dépend pas de ButtonLib.
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <vector>
#include "Button.h" // Nécessaire uniquement pour attachButtons()

extern "C" int ets_printf(const char* fmt, ...);

class SDBrowser {
public:
    // Utilise le bus SPI par défaut de la carte (broches SPI standard).
    explicit SDBrowser(uint8_t csPin);

    // Utilise un bus SPI dédié avec des broches personnalisées
    // (ex: quand SCK/MISO/MOSI ne sont pas les broches SPI par défaut).
    SDBrowser(uint8_t csPin, int8_t sck, int8_t miso, int8_t mosi, uint8_t spiBus = VSPI);

    ~SDBrowser();

    // Initialise le SPI (si personnalisé) et la carte SD, liste et affiche
    // le contenu de la racine. Retourne false si la carte n'est pas détectée.
    bool begin();

    // --- Navigation ---
    void moveUp();    // Sélectionne l'entrée précédente dans le dossier courant
    void moveDown();  // Sélectionne l'entrée suivante
    void enter();     // Entre dans le dossier sélectionné, ou affiche le fichier sélectionné
    void back();      // Remonte au dossier parent

    // Réaffiche le menu du dossier courant (ex: après un rafraîchissement manuel)
    void refresh();

    // Branche 4 boutons ButtonLib directement sur la navigation
    // (appelle button.begin() pour chacun, inutile de le refaire).
    void attachButtons(Button& up, Button& down, Button& enterBtn, Button& backBtn);

    // Nombre maximum d'octets affichés lors de la lecture d'un fichier (défaut : 2048)
    void setMaxFilePrintBytes(size_t n) { _maxFilePrintBytes = n; }

    String getCurrentPath() const { return _currentPath; }
    bool isReady() const { return _sdReady; }

private:
    void listDirectory(const String& path);
    void printMenu() const;
    void printFileContent(const String& path) const;
    static String joinPath(const String& base, const String& name);

    uint8_t   _cs;
    int8_t    _sck, _miso, _mosi;
    bool      _customSpi;
    SPIClass* _spi;
    bool      _ownsSpi;

    String _currentPath;
    std::vector<String> _entryNames;
    std::vector<bool>   _entryIsDir;
    int    _selectedIndex;
    bool   _sdReady;
    size_t _maxFilePrintBytes;
};
