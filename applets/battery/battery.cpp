#include "battery.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

QDBusConnection batteryAppletBus = QDBusConnection::systemBus();
QDBusMessage batteryAppletResponse;


QString BatteryApplet::init() {
    QString batteryName = "";

    QDir powerSupplyDir("/sys/class/power_supply");
    QStringList devices = powerSupplyDir.entryList();

    foreach (QString device, devices) {
        if (device.startsWith("BAT")) {
            batteryName = device;
            break;
        }
    }

    return batteryName;
}

Battery BatteryApplet::getBatteryState(QString batteryName) {
    // org.freedesktop.UPower
    Battery battery;

    // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp
    QString path = "/org/freedesktop/UPower/devices/battery_" + batteryName;
    QDBusInterface iface("org.freedesktop.UPower",
                         path,
                         "org.freedesktop.DBus.Properties",
                         batteryAppletBus);
    batteryAppletResponse = iface.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    foreach (QVariant v, batteryAppletResponse.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            battery.percentage = qvariant_cast<QDBusVariant>(v).variant().toInt();
        }
    }

    batteryAppletResponse = iface.call("Get", "org.freedesktop.UPower.Device", "IconName");
    foreach (QVariant v, batteryAppletResponse.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            battery.iconName = qvariant_cast<QDBusVariant>(v).variant().toString();
        }
    }

    return battery;
}
