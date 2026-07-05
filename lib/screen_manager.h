#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H


enum class Page { NOW_PLAYING, FILE_EXPLORER, SEARCH };
class ScreenManager {
public:
    void setPage(Page newPage);
    void update();  // Appelé dans loop() pour rafraîchir l'affichage
    Page getCurrentPage() const;
private:
    Page currentPage = Page::NOW_PLAYING;
    NowPlayingPage nowPlayingPage;
    FileExplorerPage fileExplorerPage;
    SearchPage searchPage;
};


#endif // SCREEN_MANAGER_H