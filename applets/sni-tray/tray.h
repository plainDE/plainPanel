#ifndef TRAY_H
#define TRAY_H

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QList>
#include <QVariant>
#include <QStringList>


class TrayApplet {
public:
    void init();
    void getTrayEntries(QStringList* trayEntriesList);
};

#endif // TRAY_H
