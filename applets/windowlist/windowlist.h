#include "../../applet.h"
#include "../../panel.h"

#include <KWindowSystem>

#ifndef WINDOWLIST_H
#define WINDOWLIST_H


namespace WindowList {
    void getWinList(QList<WId>* winIDs);
};

#endif // WINDOWLIST_H
