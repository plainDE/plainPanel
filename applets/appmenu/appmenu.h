#ifndef APPMENU_H
#define APPMENU_H

#include "../../staticapplet.h"


#include <QTabWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QProcess>

#include "../../panel.h"
#include "../../configman.h"


struct App {
    QString displayedName;
    QString exec;
    QIcon icon;
    bool display;
};

class AppMenuApplet : public StaticApplet {
public:
    AppMenuApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void internalWidgetSetup() override;
    ~AppMenuApplet();

private:
    App readDesktopEntry(QString desktopEntryPath);
    void execApp(QString exec);
    void buildMenu(QListWidget* appsList, QString filter);
    void buildFavMenu(ConfigManager* cfgMan, QListWidget* favAppsList);

    QVBoxLayout* mMainLayout;
    QTabWidget* mTabWidget;

    QWidget* mAllAppsTab;
    QVBoxLayout* mAllAppsLayout;
    QLineEdit* mSearchBox;
    QListWidget* mAppsList;

    QWidget* mFavAppsTab;
    QVBoxLayout* mFavAppsLayout;
    QListWidget* mFavAppsList;

    QWidget* mRunTab;
    QVBoxLayout* mRunLayout;
    QLabel* mRunLabel;
    QLineEdit* mCmdLineEdit;
    QPushButton* mRunPushButton;

    QHash<QListWidgetItem*, QString> mExecByItem;
};

#endif // APPMENU_H
