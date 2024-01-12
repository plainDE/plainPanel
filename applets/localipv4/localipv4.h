#ifndef LOCALIP_H
#define LOCALIP_H

#include "../../applet.h"

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>

#include <QNetworkInterface>
#include <QList>
#include <QDebug>

class LocalIPv4Applet : public Applet {
public:
    LocalIPv4Applet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    ~LocalIPv4Applet();

    QGraphicsView* mExternalWidget;

private:
    QString getLocalIP(QString ifname);

    QNetworkInterface mNetIface;
    QList<QNetworkAddressEntry> mAddrEntries;
    QGraphicsTextItem* mTextItem;
    QString mLastIP;
    int mIPAngle;

    int mInterval;
    QTimer* mTimer;
};

#endif // LOCALIP_H
