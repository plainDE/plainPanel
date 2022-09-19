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
    menuUI __createUI__(QWidget* parent, PanelLocation location, short panelHeight, QFont font,
                        short buttonX, short buttonXRight, bool triangularTabs,
                        QString accent, QString theme, qreal opacity);
    void makeItem(QString name, QIcon icon, QListWidgetItem* item);
    App readDesktopFile(QString pathToCurrentDesktopFile);
    void buildMenu(QListWidget* appList, QString searchRequest);
    void searchApps(QListWidget* menuAppsList, QString mask);
    void execApp(QWidget* parent, QString exec, QWidget* appMenuWidget);

    void buildFavMenu(QListWidget* favListWidget, QVariantList favDesktopFiles);
};

#endif // APPMENU_H
