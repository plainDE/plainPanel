#ifndef CLIOUTPUTAPPLET_H
#define CLIOUTPUTAPPLET_H

#include "../../dynamicapplet.h"

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


class CLIOutputApplet : public DynamicApplet {
public:
    CLIOutputApplet(ConfigManager* cfgMan,
                    Panel* parentPanel,
                    QString appletName);
    void externalWidgetSetup() override;
    void repeatingAction() override;
    ~CLIOutputApplet();

private:
    void readConfig();
    void setData();

    QString mAppletName;
    QJsonObject mAppletConfig;
    QProcess* mProcess;
    QString mCommand;
    QStringList mWaitData;
    QString mAppletType;
};

#endif // CLIOUTPUTAPPLET_H
