#ifndef SNITRAY_H
#define SNITRAY_H

#include "../../staticapplet.h"
#include "statusnotifierwatcher.h"

#include <QPushButton>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QThread>
#include <KWindowSystem>

class SNITrayApplet : public StaticApplet {
public:
    SNITrayApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    ~SNITrayApplet();

private:
    void setSNIIcon(QString service, QPushButton* sniPushButton);
    void activate(QString service);

    StatusNotifierWatcher* mStatusNotifierWatcher;
    QHash<QString,QPushButton*> mSNIWidgets;
};

#endif // SNITRAY_H
