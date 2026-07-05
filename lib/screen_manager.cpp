#include "screen_manager.h"

void ScreenManager::update() {
    switch (currentPage) {
        case Page::NOW_PLAYING: nowPlayingPage.render(); break;
        case Page::FILE_EXPLORER: fileExplorerPage.render(); break;
        case Page::SEARCH: searchPage.render(); break;
    }
}

