#include "workspaces.h"

void Workspaces::setCurrentSpace(qint8 newSpace) {
    KWindowSystem::setCurrentDesktop(newSpace);
}
