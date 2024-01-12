#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "../../applet.h"

#include <QPushButton>
#include <QString>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QIcon>
#include <QPropertyAnimation>

class LauncherApplet : public Applet {
    Q_OBJECT
public:
    LauncherApplet(ConfigManager* cfgMan,
                   Panel* parentPanel,
                   QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~LauncherApplet();

    QPushButton* mExternalWidget;

private:
    void execute(ConfigManager* cfgMan,
                 Panel* parentPanel,
                 QString cmd);

    int mIconSize;
    QString mEntry;
};

#endif // LAUNCHER_H
