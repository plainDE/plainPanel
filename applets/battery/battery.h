#ifndef BATTERY_H
#define BATTERY_H

#include <QString>
#include <QDir>

struct Battery {
    qint8 percentage;
    QString iconName;
};

class BatteryApplet {
public:
    QString init();
    Battery getBatteryState(QString batteryName);
};

#endif // BATTERY_H