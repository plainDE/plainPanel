#ifndef APPMENU_H
#define APPMENU_H

#include <QWidget>
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


struct App {
    QString displayedName;
    QString exec;
    QIcon icon;
    bool display;
};

class AppMenu : public QWidget {
public:
    AppMenu(QObject *execHolder,
            PanelLocation panelLocation,
            int panelThickness,
            int screenWidth,
            int screenHeight,
            QFont font,
            int buttonCoord1,
            int buttonCoord2,
            bool useTriangularTabs,
            QString accent,
            QString stylesheet,
            double opacity,
            QVariantList *favApps);
    void setAppletUI(QObject *execHolder,
                     PanelLocation panelLocation,
                     int panelThickness,
                     int screenWidth,
                     int screenHeight,
                     QFont font,
                     int buttonCoord1,
                     int buttonCoord2,
                     bool useTriangularTabs,
                     QString accent,
                     QString stylesheet,
                     double opacity);
    App readDesktopEntry(QString desktopEntryPath);
    void execApp(QObject *parent, QString exec);
    void buildMenu(QListWidget *appsList, QString filter);
    void buildFavMenu(QListWidget *favAppsList, QVariantList *favDesktopEntries);

    QVBoxLayout *mMainLayout;
    QTabWidget *mTabWidget;

    QWidget *mAllAppsTab;
    QVBoxLayout *mAllAppsLayout;
    QLineEdit *mSearchBox;
    QListWidget *mAppsList;

    QWidget *mFavAppsTab;
    QVBoxLayout *mFavAppsLayout;
    QListWidget *mFavAppsList;

    QWidget *mRunTab;
    QVBoxLayout *mRunLayout;
    QLabel *mRunLabel;
    QLineEdit *mCmdLineEdit;
    QPushButton *mRunPushButton;

    QHash<QListWidgetItem*, QString> mExecByItem;
    QVariantList *mFavApps;

    ~AppMenu();

signals:

};

#endif // APPMENU_H
