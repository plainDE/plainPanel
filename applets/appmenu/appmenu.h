#include <stdio.h>
#include <QProcess>

#include "../../applet.h"
#include "../../panel.h"

#ifndef APPMENU_H
#define APPMENU_H

/*struct App {
    QString desktopFilePath;
    QString displayedName;
    QIcon icon;
    QString exec;
};*/

struct menuUI {
    QWidget* menuWidget;
    QLineEdit* searchBox;
    QListWidget* appListWidget;
    QListWidget* favListWidget;
    QTabWidget* tabWidget;
};

struct App {
    QString displayedName;
    QString exec;
    QIcon icon;
    bool display;
};


class AppMenu : public Applet {
public:
    QString __appletName__ = "Application Menu";
    AppletType __appletType__ = appMenu;
    bool __needsToBeUpdated__ = false;
    menuUI __createUI__(PanelLocation location, short panelHeight, QFont font, short buttonX);

    void makeItem(QString name, QIcon icon, QListWidgetItem* item);
    App readDesktopFile(QString pathToCurrentDesktopFile);
    void buildMenu(QListWidget* appList);
    void searchApps(QListWidget* menuAppsList, QString mask);
    void execApp(QString exec, QWidget* appMenuWidget);

    void buildFavMenu(QListWidget* favListWidget, QVariantList favDesktopFiles);

public slots:
    void filterAppList();
};

#endif // APPMENU_H