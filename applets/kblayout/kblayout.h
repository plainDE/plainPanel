#include <QString>
#include <X11/XKBlib.h>

#ifndef KBLAYOUT_H
#define KBLAYOUT_H

// we need this to use X11 library
// seems like a bug of somewhat
#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted


namespace kblayoutApplet {
    /*extern Display* display;
    extern XkbDescPtr keyboard;
    extern XkbStateRec state;
    extern int result;
    extern QString kbLayout;*/

    void initApplet();
    QString getCurrentKbLayout();
    QString convertToISOCode(QString kbLayout);
}

#endif // KBLAYOUT_H
