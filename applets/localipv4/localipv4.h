#ifndef LOCALIP_H
#define LOCALIP_H

#include "../../dynamicapplet.h"

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>

#include <QNetworkInterface>
#include <QList>
#include <QDebug>

class LocalIPv4Applet : public DynamicApplet {
public:
    LocalIPv4Applet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void repeatingAction() override;
    ~LocalIPv4Applet();

private:
    QString getLocalIP(QString ifname);

    QNetworkInterface mNetIface;
    QList<QNetworkAddressEntry> mAddrEntries;
    QGraphicsTextItem* mTextItem;
    QString mLastIP;
    int mIPAngle;
};

#endif // LOCALIP_H
