#include <QProcess>

#ifndef VOLUME_H
#define VOLUME_H

enum VolumeAdjustMethod {
    ALSA,
    PulseAudio
};

namespace VolumeApplet {
    void setVolume(int newVolume, VolumeAdjustMethod method);
}

#endif // VOLUME_H
