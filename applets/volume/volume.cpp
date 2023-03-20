#include "volume.h"

#include <iostream>


// We use ALSA to adjust system volume.

void VolumeApplet::setVolume(int newVolume) {
    QString exec = "amixer -q -D default sset Master " + QString::number(newVolume) + "%";

    QProcess* process = new QProcess;
    process->start(exec);
}
