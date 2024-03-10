#include "../../dynamicapplet.h"

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


class KbLayoutApplet : public DynamicApplet {
public:
    KbLayoutApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void internalWidgetSetup() override;
    void repeatingAction() override;
    void repeatingAction(bool);
    void activate() override;
    ~KbLayoutApplet();

private:
    void connectToXServer();
    void cacheFlagIcons();
    void setISOCodes();
    QString getCurrentLayoutISOCode();
    void setxkbmap(QString layouts, QString toggleMethod);
    void setLayouts();
    void setChooserIndication();
    void resetChooserIndication();

    Display* mKbDisplay;
    XkbDescPtr mKeyboard;
    XkbStateRec mState;
    int mDisplayResult;
    QString mLayout;
    QString mLayoutCode;
    QJsonObject mLayoutCodes;
    QHash<QString,QIcon> mFlagByCode;
    QHash<QString,QPushButton*> mButtonByCode;
};

#endif // KBLAYOUT_H
