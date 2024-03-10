#ifndef DYNAMICAPPLET_H
#define DYNAMICAPPLET_H

#include "applet.h"

class DynamicApplet : public Applet {
public:
    DynamicApplet(QString appletID,
                  ConfigManager* cfgMan,
                  Panel* parentPanel,
                  int interval);

    virtual void activate();

    int mInterval;
    QTimer* mTimer;

public slots:
    virtual void repeatingAction();

};

#endif // DYNAMICAPPLET_H
