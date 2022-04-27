#include "../../applet.h"
#include "../../panel.h"

#include <KWindowSystem>

#ifndef WINDOWLIST_H
#define WINDOWLIST_H


class WindowList
{
public:
    QList<WId> getWinList();
};

#endif // WINDOWLIST_H
