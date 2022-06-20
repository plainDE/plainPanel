#include "kblayout.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

Display* kbDisplay;
XkbDescPtr keyboard;
XkbStateRec state;
int displayResult;
QString layout;
QJsonObject layoutCodes;


void KbLayoutApplet::__init__() {
    // Connect to X Server
    kbDisplay = XkbOpenDisplay(getenv("DISPLAY"), NULL, NULL, NULL, NULL, &displayResult);
    keyboard = XkbAllocKeyboard();

    QFile file;
    QString data;

    file.setFileName("/usr/share/plainDE/layouts.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data = file.readAll();
    file.close();

    layoutCodes = QJsonDocument::fromJson(data.toUtf8()).object();
}

QString KbLayoutApplet::getCurrentKbLayout() {
    // Obtain symbolic names from the server
    XkbGetNames(kbDisplay, XkbGroupNamesMask, keyboard);
    XkbGetState(kbDisplay, XkbUseCoreKbd, &state);

    layout = XGetAtomName(kbDisplay, keyboard->names->groups[state.group]);
    return layoutCodes[layout].toString();
}

QString KbLayoutApplet::getCurrentFlag() {
    return "/usr/share/flags/" + KbLayoutApplet::getCurrentKbLayout() + ".png";
}
