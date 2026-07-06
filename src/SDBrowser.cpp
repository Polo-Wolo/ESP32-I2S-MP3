#include "SDBrowser.h"

SDBrowser::SDBrowser(uint8_t csPin)
    : _cs(csPin), _sck(-1), _miso(-1), _mosi(-1),
      _customSpi(false), _spi(&SPI), _ownsSpi(false),
      _currentPath("/"), _selectedIndex(0), _sdReady(false),
      _maxFilePrintBytes(2048)
{
}

SDBrowser::SDBrowser(uint8_t csPin, int8_t sck, int8_t miso, int8_t mosi, uint8_t spiBus)
    : _cs(csPin), _sck(sck), _miso(miso), _mosi(mosi),
      _customSpi(true), _spi(nullptr), _ownsSpi(true),
      _currentPath("/"), _selectedIndex(0), _sdReady(false),
      _maxFilePrintBytes(2048)
{
    _spi = new SPIClass(spiBus);
}

SDBrowser::~SDBrowser() {
    if (_ownsSpi && _spi) {
        delete _spi;
        _spi = nullptr;
    }
}

bool SDBrowser::begin() {
    if (_customSpi) {
        _spi->begin(_sck, _miso, _mosi, _cs);
    } else {
        _spi->begin();
    }

    _sdReady = SD.begin(_cs, *_spi);

    if (!_sdReady) {
        ets_printf("Erreur : carte SD non detectee.\n");
        return false;
    }

    ets_printf("Carte SD initialisee.\n");
    _currentPath = "/";
    listDirectory(_currentPath);
    printMenu();
    return true;
}

String SDBrowser::joinPath(const String& base, const String& name) {
    if (base == "/") return "/" + name;
    return base + "/" + name;
}

void SDBrowser::listDirectory(const String& path) {
    _entryNames.clear();
    _entryIsDir.clear();
    _selectedIndex = 0;

    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
        ets_printf("Impossible d'ouvrir le dossier: %s\n", path.c_str());
        return;
    }

    File entry = dir.openNextFile();
    while (entry) {
        String name = String(entry.name());
        // Selon la version de la lib SD, name() peut renvoyer le chemin complet.
        // On ne garde que le dernier segment.
        int lastSlash = name.lastIndexOf('/');
        if (lastSlash >= 0) name = name.substring(lastSlash + 1);

        _entryNames.push_back(name);
        _entryIsDir.push_back(entry.isDirectory());
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();
}

void SDBrowser::printMenu() const {
    ets_printf("\n---- %s ----\n", _currentPath.c_str());
    if (_entryNames.empty()) {
        ets_printf("(dossier vide)\n");
        return;
    }
    for (size_t i = 0; i < _entryNames.size(); i++) {
        const char* cursor = ((int)i == _selectedIndex) ? ">" : " ";
        const char* suffix = _entryIsDir[i] ? "/" : "";
        ets_printf("%s %s%s\n", cursor, _entryNames[i].c_str(), suffix);
    }
}

void SDBrowser::printFileContent(const String& path) const {
    File f = SD.open(path);
    if (!f) {
        ets_printf("Impossible d'ouvrir le fichier: %s\n", path.c_str());
        return;
    }
    ets_printf("\n---- Contenu de %s (%u octets) ----\n", path.c_str(), (unsigned)f.size());

    const size_t bufSize = 128;
    char buf[bufSize + 1];
    size_t totalRead = 0;

    while (f.available() && totalRead < _maxFilePrintBytes) {
        size_t n = f.readBytes(buf, bufSize);
        buf[n] = '\0';
        ets_printf("%s", buf);
        totalRead += n;
    }
    if (f.available()) {
        ets_printf("\n... (fichier tronque a %u octets pour l'affichage)\n", (unsigned)_maxFilePrintBytes);
    }
    ets_printf("\n---- Fin du fichier ----\n");
    f.close();
}

void SDBrowser::moveUp() {
    if (!_sdReady || _entryNames.empty()) return;
    _selectedIndex--;
    if (_selectedIndex < 0) _selectedIndex = (int)_entryNames.size() - 1;
    printMenu();
}

void SDBrowser::moveDown() {
    if (!_sdReady || _entryNames.empty()) return;
    _selectedIndex++;
    if (_selectedIndex >= (int)_entryNames.size()) _selectedIndex = 0;
    printMenu();
}

void SDBrowser::enter() {
    if (!_sdReady || _entryNames.empty()) return;
    String target = joinPath(_currentPath, _entryNames[_selectedIndex]);

    if (_entryIsDir[_selectedIndex]) {
        _currentPath = target;
        listDirectory(_currentPath);
        printMenu();
    } else {
        printFileContent(target);
    }
}

void SDBrowser::back() {
    if (!_sdReady) return;
    if (_currentPath == "/") {
        ets_printf("\nDeja a la racine.\n");
        return;
    }
    int lastSlash = _currentPath.lastIndexOf('/');
    _currentPath = (lastSlash == 0) ? "/" : _currentPath.substring(0, lastSlash);
    listDirectory(_currentPath);
    printMenu();
}

void SDBrowser::refresh() {
    listDirectory(_currentPath);
    printMenu();
}

// void SDBrowser::attachButtons(Button& up, Button& down, Button& enterBtn, Button& backBtn) {
//     up.onPress([this]() { this->moveUp(); });
//     down.onPress([this]() { this->moveDown(); });
//     enterBtn.onPress([this]() { this->enter(); });
//     backBtn.onPress([this]() { this->back(); });

//     up.begin();
//     down.begin();
//     enterBtn.begin();
//     backBtn.begin();
// }
