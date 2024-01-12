#ifndef CLIOUTPUTAPPLET_H
#define CLIOUTPUTAPPLET_H

#include "../../applet.h"

#include <QObject>
#include <QPushButton>
#include <QString>
#include <QList>
#include <QProcess>
#include <QTimer>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


class CLIOutputApplet : public Applet {
public:
    CLIOutputApplet(ConfigManager* cfgMan,
                    Panel* parentPanel,
                    QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    ~CLIOutputApplet();

    QPushButton* mExternalWidget;

private:
    void readConfig();
    void setData();

    QString mAppletName;
    QJsonObject mAppletConfig;
    QProcess* mProcess;
    QString mCommand;
    QStringList mWaitData;
    QString mAppletType;

    int mInterval;
    QTimer* mTimer;
};

#endif // CLIOUTPUTAPPLET_H
