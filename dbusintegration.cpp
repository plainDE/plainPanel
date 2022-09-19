#include "dbusintegration.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>


DBusIntegration::DBusIntegration(QString service,
                                 QString path,
                                 QString interfaceName,
                                 QObject* parent) : QDBusAbstractAdaptor(parent) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerService(service);
    bus.registerObject(path,
                       interfaceName,
                       parent,
                       QDBusConnection::ExportAllSlots);
}
