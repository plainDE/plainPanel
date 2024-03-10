#ifndef STATICAPPLET_H
#define STATICAPPLET_H

#include "applet.h"

class StaticApplet : public Applet {
public:
    StaticApplet(QString appletID,
                 ConfigManager* cfgMan,
                 Panel* parentPanel);
};

#endif // STATICAPPLET_H
