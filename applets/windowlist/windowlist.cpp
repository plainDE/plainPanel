#include "windowlist.h"
#include <KWindowSystem>

QList<WId> WindowList::getWinList() {
    QList<WId> windows = KWindowSystem::windows();
    return windows;
}
