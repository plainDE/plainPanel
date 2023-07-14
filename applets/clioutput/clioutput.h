#ifndef CLIOUTPUTAPPLET_H
#define CLIOUTPUTAPPLET_H

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

class CLIOutputApplet : public QPushButton {
    Q_OBJECT

public:
    void readConfig(QString appletName);
    void setData();
    void getData();
    void activate();
    CLIOutputApplet(QObject* parent,
                    QString appletName);
    ~CLIOutputApplet();

    QProcess* mProcess;
    QString mCommand;
    QString mAppletType;
    QTimer* mTimer;
    QStringList mWaitData;

private:
    QJsonObject mAppletConfig;
};

#endif // CLIOUTPUTAPPLET_H
