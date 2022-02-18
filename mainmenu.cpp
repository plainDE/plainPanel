#include <QString>
#include <QList>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "mainmenu.h"
#include "ui_mainmenu.h"

mainMenu::mainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainMenu)
{
    ui->setupUi(this);
}

struct App {
    QString desktopFileName;
    QString name;
    QString icon;
    QString exec;
};

QList<App> setAppsList(Ui::mainMenu* ui) {
    QList<App> apps;
    ui->appsListWidget->clear();
    QDir desktopsDir = QDir("/usr/share/applications/");
    QString tmpName, tmpIcon, tmpExec;
    bool setName = false, setIcon = false, setExec = false;  // we need these vars not to allow overwriting values from desktop file
    int equalIndex = 0;
    foreach (QString file, desktopsDir) {
        QFile desktopFile(file);
        desktopFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream streamIn(&desktopFile);

        while (!streamIn.atEnd()) {
            QString line = streamIn.readLine();

            if (line.startsWith("Name")) {
                if (!setName) {
                    tmpName = line.split('=')[1];
                    while (tmpName.startsWith(' ')) {
                        tmpName.truncate(1);
                    }
                    setName = true;
                }
            }
            else if (line.startsWith("Icon")) {
                if (!setIcon) {
                    tmpIcon = line.split('=')[1];
                    while (tmpIcon.startsWith(' ')) {
                        tmpIcon.truncate(1);
                    }
                    setIcon = true;
                }
            }
            else if (line.startsWith("Exec")) {
                if (!setExec) {
                    tmpExec = line.split('=')[1];
                    while (tmpExec.startsWith(' ')) {
                        tmpExec.truncate(1);
                    }
                    setExec = true;
                }
            }
        }

        apps << App{file, tmpName, tmpIcon, tmpExec};

    }

    return apps;
}

mainMenu::~mainMenu()
{
    delete ui;
}
