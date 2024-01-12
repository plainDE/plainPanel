#ifndef USERMENU_H
#define USERMENU_H

#include "../../applet.h"

#include <QWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QFile>


class UserMenuApplet : public Applet {
    Q_OBJECT

public:
    UserMenuApplet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void internalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~UserMenuApplet();

    QPushButton* mExternalWidget;
    QWidget* mInternalWidget;


signals:
    void panelShouldQuit();
};

#endif // USERMENU_H
