#include "workspaces.h"

void Workspaces::setCurrentSpace(qint8 newSpace) {
    KX11Extras::setCurrentDesktop(newSpace);
}
