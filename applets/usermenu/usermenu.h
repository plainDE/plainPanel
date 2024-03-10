#ifndef USERMENU_H
#define USERMENU_H

#include "../../staticapplet.h"
#include "../../initializer.h"

#include <QWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QFile>


class UserMenuApplet : public StaticApplet {
    Q_OBJECT

public:
    UserMenuApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void internalWidgetSetup() override;
    ~UserMenuApplet();

signals:
    void panelShouldQuit();
};

#endif // USERMENU_H
