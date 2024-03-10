#ifndef SPLITTERAPPLET_H
#define SPLITTERAPPLET_H

#include <staticapplet.h>

class SplitterApplet : public StaticApplet {
public:
    SplitterApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    ~SplitterApplet();
};

#endif // SPLITTERAPPLET_H
