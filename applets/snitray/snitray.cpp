#include "snitray.h"

QDBusConnection mSNITrayBus = QDBusConnection::sessionBus();

void SNITrayApplet::externalWidgetSetup() {
    mExternalWidget = new QFrame();
    static_cast<QFrame*>(mExternalWidget)->setFrameStyle(
        QFrame::NoFrame | QFrame::Plain
    );

    QBoxLayout* layout;
    if (mParentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
    }
    else {  // Vertical
        layout = new QVBoxLayout(mExternalWidget);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(mParentPanel->mSpacing);

    // Make connections
    connect(mStatusNotifierWatcher, &StatusNotifierWatcher::StatusNotifierItemRegistered,
            this, [this, layout]() {
        QString service = mStatusNotifierWatcher->registeredStatusNotifierItems().constLast();
        qDebug() << "Adding icon to SNI layout..." << service;

        QPushButton* sniPushButton = new QPushButton();
        sniPushButton->setFlat(true);
        sniPushButton->setIcon(QIcon::fromTheme("dialog-question"));
        mSNIWidgets[service] = sniPushButton;
        layout->addWidget(sniPushButton);

        // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp
        QDBusInterface iface(service, "/StatusNotifierItem", "org.freedesktop.DBus.Properties");
        QDBusMessage response = iface.call("Get", "org.kde.StatusNotifierItem", "ToolTip");
        QString tooltip;
        foreach (QVariant v, response.arguments()) {
            if (v.userType() == qMetaTypeId<QDBusVariant>()) {
                tooltip = qvariant_cast<QDBusVariant>(v).variant().toString();
                break;
            }
        }
        if (tooltip.isEmpty()) {
            tooltip = service;
        }

        QtConcurrent::run(this, &SNITrayApplet::setSNIIcon, service, sniPushButton);

        this->connect(sniPushButton, &QPushButton::clicked, this, [this, service]() {
            qDebug() << "Activate";
            QtConcurrent::run(this, &SNITrayApplet::activate, service);
        });
    });

    this->connect(mStatusNotifierWatcher, &StatusNotifierWatcher::StatusNotifierItemUnregistered,
                  this, [this]() {
        QString service = mStatusNotifierWatcher->deletedItems.last();
        delete mSNIWidgets[service];
        mSNIWidgets.remove(service);
    });
}

void SNITrayApplet::setSNIIcon(QString service, QPushButton* sniPushButton) {
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

        QDBusConnectionInterface* connIface = mSNITrayBus.interface();
        QDBusReply<uint> servicePID = connIface->servicePid(service);

        QString path = QString("/proc/%1/exe").arg(servicePID);
        QString pathToExec = QFile::symLinkTarget(path);
        QFileInfo info(pathToExec);
        QString workDirPath = info.dir().absolutePath();

        QIcon icon;

        if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
            icon = QIcon::fromTheme(iconName);
        }
        else if (!iconName.isEmpty()) {
            icon = QIcon(QString("%1/%2").arg(workDirPath, iconName));
        }

        if (!icon.isNull()) {
            sniPushButton->setIcon(icon);
        }
        else {  // If there is no icon by IconName, we try to get IconPixmap
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
                // If there is no IconPixmap, this is the last chance
                // Let's try to get icon by WinID of process that runs a D-Bus service

                qDebug() << "no iconpixmap";
                QDBusConnectionInterface* connIface = mSNITrayBus.interface();
                QDBusReply<uint> servicePID = connIface->servicePid(service);
                qDebug() << "servicePID:" << servicePID;

                bool found = false;
                for (int i = 0; i < 5; ++i) {
                    found = false;
                    QList<WId> winIDs = KWindowSystem::windows();
                    for (auto it = winIDs.cbegin(), end = winIDs.cend(); it != end; ++it) {
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
                    QThread::sleep(2);
                }

                if (!found) {
                    sniPushButton->setIcon(QIcon::fromTheme("dialog-question"));
                }
            }
        }
    }
    else {
        qDebug() << "Seems that" << service << "is not a valid D-Bus service.";
    }
}

void SNITrayApplet::activate(QString service) {
    QDBusMessage message = QDBusMessage::createMethodCall(service,
                                                          "/StatusNotifierItem",
                                                          "org.kde.StatusNotifierItem",
                                                          "Activate");
    QList<QVariant> args;
    args.append(0);
    args.append(0);

    message.setArguments(args);
    mSNITrayBus.call(message);
}

SNITrayApplet::SNITrayApplet(ConfigManager* cfgMan,
                             Panel* parentPanel) : StaticApplet(
                                                       "org.plainDE.sniTray",
                                                       cfgMan,
                                                       parentPanel
                                                   ) {
    mStatusNotifierWatcher = new StatusNotifierWatcher();
    mStatusNotifierWatcher->RegisterStatusNotifierHost("org.plainDE.plainPanel");
}

SNITrayApplet::~SNITrayApplet() {
    delete mStatusNotifierWatcher;

    foreach (QPushButton* button, mSNIWidgets.values()) {
        delete button;
    }

    delete mExternalWidget;
}
