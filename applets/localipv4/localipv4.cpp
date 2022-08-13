#include "localipv4.h"

QNetworkInterface netIface;
QList<QNetworkAddressEntry> addrEntries;

QString LocalIPv4Applet::getLocalIP(QString ifname) {
    netIface = QNetworkInterface::interfaceFromName(ifname);
    addrEntries = netIface.addressEntries();

    if (!addrEntries.isEmpty()) {
        return addrEntries.at(0).ip().toString();
    }
    else {
        return "0.0.0.0";
    }
}
