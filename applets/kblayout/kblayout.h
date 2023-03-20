#include <QIcon>
#include "../../panel.h"

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


class KbLayoutApplet {
public:
    void __init__(QStringList activeLayouts);
    QString getCurrentKbLayout();
    QIcon getCurrentFlag();
};

#endif // KBLAYOUT_H
