#include "kblayout.h"

#include <QString>
#include <QDebug>
#include <QInputMethod>
#include <QMap>

// https://github.com/luminousmen/xkblang


Display* display;
XkbDescPtr keyboard;
XkbStateRec	state;
int result;
QString kbLayout;
QMap<QString,QString> codes;


void kblayoutApplet::initApplet() {
    // Connect to X Server
    display = XkbOpenDisplay(getenv("DISPLAY"), NULL, NULL, NULL, NULL, &result);
    keyboard = XkbAllocKeyboard();

    // Init supported ISO codes
    codes.insert("du", "nl");
    codes.insert("en", "us");
    codes.insert("ge", "de");
    codes.insert("sp", "es");
    codes.insert("uk", "ua");
}

QString kblayoutApplet::getCurrentKbLayout() {
    // Obtain symbolic names from the server
    XkbGetNames(display, XkbGroupNamesMask, keyboard);
    XkbGetState(display, XkbUseCoreKbd, &state);

    kbLayout = XGetAtomName(display, keyboard->names->groups[state.group]);
    kbLayout.truncate(2);

    XkbFreeNames(keyboard, XkbGroupNamesMask, True);
    //qDebug() << 1;

    return kbLayout.toLower();
}

QString kblayoutApplet::convertToISOCode(QString kblayout) {
    return codes.value(kblayout, kblayout);
}

