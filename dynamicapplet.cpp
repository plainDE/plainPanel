#include "dynamicapplet.h"

void DynamicApplet::activate() {
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    connect(mTimer, &QTimer::timeout, this, &DynamicApplet::repeatingAction);
    mTimer->start();
    repeatingAction();
}

void DynamicApplet::repeatingAction() {

}

DynamicApplet::DynamicApplet(QString appletID,
                             ConfigManager* cfgMan,
                             Panel* parentPanel,
                             int interval) : Applet(appletID,
                                                    cfgMan,
                                                    parentPanel) {
    mAppletType = Dynamic;
    mInterval = interval;
}
