#include "volume.h"

#include <cstdlib>
#include <iostream>

void VolumeApplet::setVolume(int newVolume) {
    QString exec = "amixer -q -D default sset Master " + QString::number(newVolume);

    QProcess* process = new QProcess;
    process->start(exec);
}
