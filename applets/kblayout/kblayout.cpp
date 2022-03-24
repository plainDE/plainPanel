#include "kblayout.h"

Display* kbDisplay;
XkbDescPtr keyboard;
XkbStateRec state;
int displayResult;
QString layout;


void KbLayoutApplet::__init__() {
    // Connect to X Server
    kbDisplay = XkbOpenDisplay(getenv("DISPLAY"), NULL, NULL, NULL, NULL, &displayResult);
    keyboard = XkbAllocKeyboard();
}

QString KbLayoutApplet::getCurrentKbLayout() {
    // Obtain symbolic names from the server
    XkbGetNames(kbDisplay, XkbGroupNamesMask, keyboard);
    XkbGetState(kbDisplay, XkbUseCoreKbd, &state);

    layout = XGetAtomName(kbDisplay, keyboard->names->groups[state.group]);
    layout = layout.toLower();
    layout.truncate(2);

    return layout;
}
