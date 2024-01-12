#ifndef VOLUME_H
#define VOLUME_H

#include "../../applet.h"

#include <QFrame>
#include <QProcess>

class VolumeApplet : public Applet {
public:
    VolumeApplet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~VolumeApplet();
    void setVolume(int newVolume, VolumeAdjustMethod method);

    QFrame* mExternalWidget;
};

#endif // VOLUME_H
