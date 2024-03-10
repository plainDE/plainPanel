#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include "../../dynamicapplet.h"

#include <KWindowSystem>

class WindowListApplet : public DynamicApplet {
public:
    WindowListApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void activate() override;
    void addButtons();
    void accentActiveWindow();
    void updateWinTitlesLength();
    ~WindowListApplet();

    //QBoxLayout* mExternalLayout;

private:
    void getWinList();
    int getTitleSize(QString title);
    QString shortenTitle(QPushButton* button, QString title);
    void repeatingAction() override;
    void removeOldButtons();

    int mIconSize;
    QList<WId> mWIDList;
    QHash<WId, QPushButton*> mWinWidgets;
    QHash<WId,QString> mFullTitleByWId;
    QHash<WId,QString> mCurrentTitleByWId;
    QHash<WId,int> mButtonSizeByWId;
};

#endif // WINDOWLIST_H
