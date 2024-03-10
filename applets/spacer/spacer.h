#ifndef SPACERAPPLET_H
#define SPACERAPPLET_H

#include <staticapplet.h>

class SpacerApplet : public StaticApplet {
public:
    SpacerApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    ~SpacerApplet();
};

#endif // SPACERAPPLET_H
