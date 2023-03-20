#ifndef SNITRAY_H
#define SNITRAY_H

#include "statusnotifierwatcher.h"
#include "../windowlist/windowlist.h"

#include <QPushButton>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QThread>


class SNITray {
public:
    SNITray();
    ~SNITray();
    void init();
    void setSNIIcon(QString service, QPushButton* sniPushButton);

    StatusNotifierWatcher* mStatusNotifierWatcher;
    QList<WId>* mWinIDs;
};

#endif // SNITRAY_H
