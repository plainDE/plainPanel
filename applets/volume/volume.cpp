#include "volume.h"

#include <iostream>


// We use ALSA to adjust system volume.

void VolumeApplet::setVolume(int newVolume, VolumeAdjustMethod method) {
    QString exec;
    if (method == ALSA) {
        exec = "amixer -q -D default sset Master " + QString::number(newVolume) + "%";
    }
    else if (method == PulseAudio) {
        exec = "pactl set-sink-volume @DEFAULT_SINK@ " + QString::number(newVolume) + "%";
    }

    QProcess* process = new QProcess;
    process->start(exec);
}
