#include "windowlist.h"

void WindowList::getWinList(QList<WId>* winIDs) {
    *winIDs = KWindowSystem::windows();
}
