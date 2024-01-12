#ifndef SNITRAY_H
#define SNITRAY_H

#include "../../applet.h"
#include "statusnotifierwatcher.h"

#include <QPushButton>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QThread>
#include <KWindowSystem>

class SNITrayApplet : public Applet {
public:
    SNITrayApplet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~SNITrayApplet();

    QFrame* mExternalWidget;

private:
    void setSNIIcon(QString service, QPushButton* sniPushButton);
    void activate(QString service);

    StatusNotifierWatcher* mStatusNotifierWatcher;
    QHash<QString,QPushButton*> mSNIWidgets;
};

#endif // SNITRAY_H
