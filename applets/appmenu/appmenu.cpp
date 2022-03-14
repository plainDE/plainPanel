#include <QDebug>

#include "appmenu.h"
#include "../../panel.h"

QHash<QListWidgetItem*,QString> execData;

menuUI AppMenu::__createUI__(PanelLocation location, short panelHeight, QFont font, short buttonX) {
    QWidget* appMenuWidget = new QWidget;

    // Window flags
    // TODO: Menu should not show up in Alt-Tab (do not use Qt::X11BypassWindowManagerHint)
    appMenuWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);


    // Geometry
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    short menuWidth = 450;
    short menuHeight = primaryScreen->geometry().height() / 2;
    short ax = 0, ay = 0;
    if (location == top) {
        ay = panelHeight + 5;
    }
    else {
        ay = primaryScreen->geometry().height() - panelHeight - menuHeight - 5;
    }
    if (primaryScreen->geometry().width() - buttonX >= menuWidth) {
        ax = buttonX;
    }
    else {
        ax = buttonX - menuWidth;
    }
    appMenuWidget->setFixedSize(menuWidth, menuHeight);
    appMenuWidget->move(ax, ay);


    // UI
    appMenuWidget->setFont(font);

    QVBoxLayout* menuLayout = new QVBoxLayout;
    menuLayout->setContentsMargins(4, 4, 4, 4);
    appMenuWidget->setLayout(menuLayout);

    QLineEdit* menuSearchBox = new QLineEdit;
    menuSearchBox->setPlaceholderText("🔎 Search");  // u+01f50e - magnifier icon
    menuSearchBox->setClearButtonEnabled(true);
    appMenuWidget->layout()->addWidget(menuSearchBox);

    QListWidget* menuAppsList = new QListWidget;
    appMenuWidget->layout()->addWidget(menuAppsList);

    appMenuWidget->connect(menuSearchBox, &QLineEdit::textEdited, appMenuWidget,
                           [menuAppsList, menuSearchBox, this]() {
        searchApps(menuAppsList, menuSearchBox->text());
    });

    appMenuWidget->connect(menuAppsList, &QListWidget::itemDoubleClicked, appMenuWidget,
                           [menuAppsList, appMenuWidget, this]() {
        execApp(execData[menuAppsList->selectedItems()[0]], appMenuWidget);
    });


    return {appMenuWidget, menuSearchBox, menuAppsList};
}

App AppMenu::readDesktopFile(QString pathToCurrentDesktopFile) {
    App myApp;
    QString iconPath;

    QSettings desktopFileReader(pathToCurrentDesktopFile, QSettings::IniFormat);
    desktopFileReader.sync();
    desktopFileReader.beginGroup("Desktop Entry");
        if (!desktopFileReader.value("NoDisplay").toBool()) {
            myApp.display = true;
            myApp.displayedName = desktopFileReader.value("Name").toString();
            myApp.exec = desktopFileReader.value("Exec").toString();
            iconPath = desktopFileReader.value("Icon").toString();
            if (QIcon::hasThemeIcon(iconPath)) {
                myApp.icon = QIcon::fromTheme(iconPath);
            }
            else {
                if (QFile::exists(iconPath)) {
                    myApp.icon = QIcon(iconPath);
                }
                else {
                    // ICON: unknown app
                    myApp.icon = QIcon(iconPath);
                }
            }
        }
        else {
            myApp.display = false;
        }

    return myApp;
}


void AppMenu::execApp(QString exec, QWidget* appMenuWidget) {
    QProcess* process = new QProcess(appMenuWidget);

    /* https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html
     * 'The Exec key' */
    if (exec[exec.length()-2] == "%") {
        exec.chop(2);
    }
    process->start(exec);
    appMenuWidget->hide();
}


void AppMenu::buildMenu(QListWidget* menuAppsList, QList<App>* menu, QList<QString>* execList) {
    menuAppsList->clear();

    QDir appDir("/usr/share/applications");
    QStringList desktopFilesList = appDir.entryList();
    QString pathToCurrentDesktopFile;
    App currentApp;

    desktopFilesList.removeFirst();
    desktopFilesList.removeFirst();

    menu->clear();
    menuAppsList->clear();
    execList->clear();

    qDebug() << desktopFilesList.length();

    for (int i = 0; i < desktopFilesList.length(); ++i) {
        pathToCurrentDesktopFile = appDir.absoluteFilePath(desktopFilesList[i]);

        currentApp = readDesktopFile(pathToCurrentDesktopFile);
        if (currentApp.display) {
            menu->append(currentApp);
            QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
            menuAppsList->addItem(item);
            menuAppsList->setCurrentItem(item);
            menuAppsList->currentItem()->setIcon(currentApp.icon);

            execData[item] = currentApp.exec;
        }
    }

    menuAppsList->setCurrentRow(0);
}


void AppMenu::searchApps(QListWidget* menuAppsList, QString mask) {
    QDir appDir("/usr/share/applications");
    QStringList desktopFilesList = appDir.entryList();
    QString pathToCurrentDesktopFile;
    App currentApp;

    desktopFilesList.removeFirst();
    desktopFilesList.removeFirst();

    menuAppsList->clear();

    for (int i = 0; i < desktopFilesList.length(); ++i) {
        pathToCurrentDesktopFile = appDir.absoluteFilePath(desktopFilesList[i]);

        currentApp = readDesktopFile(pathToCurrentDesktopFile);
        if (currentApp.display) {
            if (currentApp.displayedName.contains(mask, Qt::CaseInsensitive)) {
                QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                menuAppsList->addItem(item);
                menuAppsList->setCurrentItem(item);
                menuAppsList->currentItem()->setIcon(currentApp.icon);
            }
        }
    }
}

