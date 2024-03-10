#ifndef VOLUME_H
#define VOLUME_H

#include "../../staticapplet.h"

#include <QFrame>
#include <QProcess>

class VolumeApplet : public StaticApplet {
public:
    VolumeApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    ~VolumeApplet();

private:
    void setVolume(int newVolume, VolumeAdjustMethod method);
};

#endif // VOLUME_H
