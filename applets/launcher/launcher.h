#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../../staticapplet.h"

#include <QPushButton>
#include <QString>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QIcon>
#include <QPropertyAnimation>

class LauncherApplet : public StaticApplet {
    Q_OBJECT
public:
    LauncherApplet(ConfigManager* cfgMan, Panel* parentPanel, QString entry);
    void externalWidgetSetup() override;
    QIcon resolveIconNameOrPath(QString iconNameOrPath);
    ~LauncherApplet();

private:
    void execute(QString cmd);

    int mIconSize;
    QString mEntry;
};

#endif // LAUNCHER_H
