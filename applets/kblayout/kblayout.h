#include "../../applet.h"

#include <QIcon>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>

#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Unsorted
#undef Bool
#undef True
#undef False

#ifndef KBLAYOUT_H
#define KBLAYOUT_H


class KbLayoutApplet : public Applet {
public:
    KbLayoutApplet(ConfigManager* cfgMan,
                   Panel* parentPanel,
                   QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan,
                         Panel* parentPanel,
                         bool useFlag);
    void repeatingAction(ConfigManager* cfgMan,
                         Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    ~KbLayoutApplet();

    QPushButton* mExternalWidget;

private:
    void connectToXServer();
    void cacheFlagIcons(ConfigManager* cfgMan);
    void setISOCodes();
    QString getCurrentLayoutISOCode();
    void setLayouts(ConfigManager* cfgMan, Panel* parentPanel);

    Display* mKbDisplay;
    XkbDescPtr mKeyboard;
    XkbStateRec mState;
    int mDisplayResult;
    QString mLayout;
    QJsonObject mLayoutCodes;
    QHash<QString,QIcon> mFlagByCode;

    int mInterval;
    QTimer* mTimer;
};

#endif // KBLAYOUT_H
