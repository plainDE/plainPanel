#ifndef BATTERY_H
#define BATTERY_H

#include "../../dynamicapplet.h"

#include <QDir>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <QFrame>


struct Battery {
    int percentage;
    QString iconName;
};

class BatteryApplet : public DynamicApplet {
public:
    BatteryApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void repeatingAction() override;
    ~BatteryApplet();

    static bool deviceHasBattery();

private:
    void cacheIcons();
    void setBatteryName();

    Battery mBattery;

    QString mBatteryName;
    QDBusMessage mResponse;

    QLabel* mIconLabel;
    QLabel* mTextLabel;
    int mIconSize;
    QHash<QString,QPixmap> mCache;
    bool mDark;

private slots:
    void updateBatteryInfo();
};

#endif // BATTERY_H
