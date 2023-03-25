#include "snitray.h"

QDBusConnection sessionBus = QDBusConnection::sessionBus();

void SNITray::init() {
    mStatusNotifierWatcher = new StatusNotifierWatcher();
    mStatusNotifierWatcher->RegisterStatusNotifierHost("org.plainDE.plainPanel");
}

void SNITray::setSNIIcon(QString service, QPushButton* sniPushButton) {
    // First of all, we try to get icon by IconName
    if (!service.contains('/')) {
        QDBusInterface iface(service, "/StatusNotifierItem", "org.freedesktop.DBus.Properties");
        QDBusMessage resp = iface.call("Get", "org.kde.StatusNotifierItem", "IconName");
        QString iconName;

        foreach (QVariant v, resp.arguments()) {
            if (v.userType() == qMetaTypeId<QDBusVariant>()) {
                iconName = qvariant_cast<QDBusVariant>(v).variant().toString();
            }
        }

        // If there is no IconName, we try to get IconPixmap
        if (!iconName.isEmpty()) {
            sniPushButton->setIcon(QIcon::fromTheme(iconName));
        }
        else {
            // We get icon byte array from service/StatusNotifierItem/org.kde.StatusNotifierItem/IconPixmap
            QDBusInterface interface(service, "/StatusNotifierItem", "org.freedesktop.DBus.Properties");
            resp = interface.call("Get", "org.kde.StatusNotifierItem", "IconPixmap");

            if (resp.type() == 2) {
                QList<QVariant> outArgs = resp.arguments();
                QVariant first = outArgs.at(0);
                QDBusVariant dbvFirst = first.value<QDBusVariant>();
                QVariant vFirst = dbvFirst.variant();
                const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

                QList<QXdgDBusImageVector> iconData;
                dbusArgs.beginArray();
                while (!dbusArgs.atEnd()) {
                    QXdgDBusImageVector curr;
                    dbusArgs.beginStructure();
                    dbusArgs >> curr.height >> curr.width >> curr.data;
                    iconData.append(curr);
                }
                dbusArgs.endArray();

                QByteArray converted;
                for (auto i = iconData.at(0).data.begin(); i != iconData.at(0).data.end() ; i += 8) {
                    QByteArray seq;
                    for (auto j = i; j != i+8; ++j) {
                        seq += *j;
                    }
                    QByteArray revSeq(seq.size(), 0);
                    std::copy(seq.crbegin(), seq.crend(), revSeq.begin());
                    converted += revSeq;
                }

                auto img = QImage((const uchar *)converted.constData(),
                                  iconData.at(0).width,
                                  iconData.at(0).height,
                                  QImage::Format_ARGB32);

                sniPushButton->setIcon(QIcon(QPixmap::fromImage(img)));
            }
            else {
                qDebug() << "no iconpixmap.";
                // If there is no IconPixmap, this is the last chance
                // Let's try to get icon by WinID of process that runs a D-Bus service
                QDBusConnectionInterface* connIface = sessionBus.interface();
                QDBusReply<uint> servicePID = connIface->servicePid(service);

                qDebug() << "servicePID:" << servicePID;


                for (int i = 0; i < 5; ++i) {
                    bool found = false;
                    WindowList::getWinList(mWinIDs);
                    for (auto it = mWinIDs->cbegin(), end = mWinIDs->cend(); it != end; ++it) {
                        KWindowInfo pIDInfo(*it, NET::WMPid);
                        qDebug() << pIDInfo.pid();
                        if ((uint)pIDInfo.pid() == servicePID.value()) {
                            sniPushButton->setIcon(KWindowSystem::icon(*it));
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                    QThread::sleep(1);
                }
            }
        }
    }
    else {
        qDebug() << "Seems that" << service << "is not a valid D-Bus service.";
    }
}

void SNITray::activate(QString service) {
    QDBusMessage message = QDBusMessage::createMethodCall(service,
                                                          "/StatusNotifierItem",
                                                          "org.kde.StatusNotifierItem",
                                                          "Activate");
    QList<QVariant> args;
    args.append(0);
    args.append(0);

    message.setArguments(args);
    sessionBus.call(message);
}

SNITray::SNITray() {
    init();
    mWinIDs = new QList<WId>;
}

SNITray::~SNITray() {
    delete mStatusNotifierWatcher;
}
