#include "windowlist.h"

QList<WId> WindowList::getWinList() {
    QList<WId> windows = KWindowSystem::windows();
    return windows;
}
