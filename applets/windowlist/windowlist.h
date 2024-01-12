#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include "../../applet.h"

#include <KWindowSystem>

class WindowListApplet : public Applet {
public:
    WindowListApplet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    void addButtons(ConfigManager* cfgMan, Panel* parentPanel);
    void accentActiveWindow(ConfigManager* cfgMan);
    void updateWinTitlesLength(Panel* parentPanel);
    ~WindowListApplet();

    QBoxLayout* mExternalLayout;

private:
    void getWinList();
    int getTitleSize(Panel* parentPanel, QString title);
    QString shortenTitle(Panel* parentPanel,
                         QPushButton* button,
                         QString title);
    void updateWinTitles(Panel* parentPanel);
    void removeOldButtons(ConfigManager* cfgMan, Panel* parentPanel);

    ConfigManager* mCfgMan;

    int mIconSize;
    QList<WId> mWIDList;
    QHash<WId, QPushButton*> mWinWidgets;
    QHash<WId,QString> mFullTitleByWId;
    QHash<WId,QString> mCurrentTitleByWId;
    QHash<WId,int> mButtonSizeByWId;

    int mInterval;
    QTimer* mUpdateTitlesTimer;
};

#endif // WINDOWLIST_H
