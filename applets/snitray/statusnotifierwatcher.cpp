#include "statusnotifierwatcher.h"

QDBusConnection sni_sessionBus = QDBusConnection::sessionBus();

StatusNotifierWatcher::StatusNotifierWatcher(QObject *parent) : QObject{parent} {
    sni_sessionBus.registerService("org.kde.StatusNotifierWatcher");
    sni_sessionBus.registerObject("/StatusNotifierWatcher",
                              this,
                              QDBusConnection::ExportAllContents);
}

void StatusNotifierWatcher::RegisterStatusNotifierItem(const QString &serviceOrPath) {
    if (!serviceOrPath.contains('/')) {
        items.append(serviceOrPath);

        QDBusServiceWatcher* watcher = new QDBusServiceWatcher(serviceOrPath,
                                                               sni_sessionBus,
                                                               QDBusServiceWatcher::WatchForUnregistration);

        this->connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this, serviceOrPath]() {
            deletedItems.append(serviceOrPath);
            emit StatusNotifierItemUnregistered(serviceOrPath);
        });

        emit StatusNotifierItemRegistered(serviceOrPath);

        qDebug() << "REGISTERED StatusNotifierItem:" << serviceOrPath;
    }
    else {
        qDebug() << "Seems that" << serviceOrPath << "is not valid D-Bus service.";
    }
}

void StatusNotifierWatcher::RegisterStatusNotifierHost(const QString &service) {
    hosts.append(service);
    emit StatusNotifierHostRegistered();
    qDebug() << "Registered StatusNotifierHost:" << service;
}

void StatusNotifierWatcher::UnregisterStatusNotifierItem(const QString &service, const QString &path) {
    qDebug() << "unREGISTERED ITEM: " << service;
}
