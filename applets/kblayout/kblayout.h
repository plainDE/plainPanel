#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

#include "../../applet.h"
#include "../../panel.h"

#ifndef KBLAYOUT_H
#define KBLAYOUT_H


class KbLayoutApplet
{
public:
    void __init__();
    QString getCurrentKbLayout();
};

#endif // KBLAYOUT_H