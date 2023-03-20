#include "../../panel.h"

#include <QNetworkInterface>
#include <QList>
#include <QDebug>

#ifndef LOCALIP_H
#define LOCALIP_H


class LocalIPv4Applet {
public:
    QString getLocalIP(QString ifname);
};

#endif // LOCALIP_H
