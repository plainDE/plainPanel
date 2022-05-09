#include "tray.h"

QDBusConnection dbusConnection = QDBusConnection::sessionBus();
QDBusMessage msg;
QList<QVariant> args = {"org.kde.StatusNotifierWatcher",
                        "RegisteredStatusNotifierItems"};


void TrayApplet::init() {
    msg = QDBusMessage::createMethodCall("org.kde.StatusNotifierWatcher",
                                         "/StatusNotifierWatcher",
                                         "org.freedesktop.DBus.Properties",
                                         "Get");
    msg.setArguments(args);
}

void TrayApplet::getTrayEntries(QStringList* trayEntriesList) {
    QDBusReply<QDBusVariant> reply = dbusConnection.call(msg);
    foreach (QString item, reply.value().variant().toStringList()) {
        trayEntriesList->append(item);
    }
}

