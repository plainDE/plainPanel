#include "staticapplet.h"

StaticApplet::StaticApplet(QString appletID,
                           ConfigManager* cfgMan,
                           Panel* parentPanel) : Applet(appletID,
                                                        cfgMan,
                                                        parentPanel) {
    mAppletType = Static;
}
