#ifndef BATTERY_H
#define BATTERY_H

#include "../../applet.h"

#include <QDir>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <QFrame>


struct Battery {
    int percentage;
    QString iconName;
};

class BatteryApplet : public Applet {
public:
    BatteryApplet(ConfigManager* cfgMan,
                  Panel* parentPanel,
                  QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    ~BatteryApplet();

    QFrame* mExternalWidget;
    bool mDeviceHasBattery;

private:
    void cacheIcons();
    void setBatteryName();
    void updateBatteryState();
    void showBatteryData();

    Battery mBattery;

    QString mBatteryName;
    QDBusMessage mResponse;

    QLabel* mIconLabel;
    QLabel* mTextLabel;
    int mIconSize;
    QHash<QString,QPixmap> mCache;
    bool mDark;

    int mInterval;
    QTimer* mTimer;

    /*QString init();
    Battery getBatteryState(QString batteryName);*/
};

#endif // BATTERY_H
