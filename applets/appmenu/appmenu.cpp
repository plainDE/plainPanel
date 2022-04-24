#include <QDebug>

#include "appmenu.h"
#include "../../panel.h"

QHash<QListWidgetItem*,QString> execData;
QHash<QListWidgetItem*,QString> favExecData;
QVariantList __favDesktopFiles__;

menuUI AppMenu::__createUI__(PanelLocation location, short panelHeight, QFont font,
                             short buttonX, short buttonXRight, bool triangularTabs) {
    QWidget* appMenuWidget = new QWidget;
    appMenuWidget->setWindowTitle("plainPanel App Menu");

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
        ax = buttonXRight - menuWidth;
    }
    appMenuWidget->setFixedSize(menuWidth, menuHeight);
    appMenuWidget->move(ax, ay);

    // UI: Menu
    appMenuWidget->setFont(font);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(4, 4, 4, 4);
    appMenuWidget->setLayout(mainLayout);

    QTabWidget* menuTabWidget = new QTabWidget;
    if (triangularTabs) {
        menuTabWidget->setTabShape(QTabWidget::Triangular);
    }
    appMenuWidget->layout()->addWidget(menuTabWidget);


    // UI: All applications tab
    QWidget* allAppsTab = new QWidget;
    allAppsTab->setFont(font);

    QVBoxLayout* menuLayout = new QVBoxLayout;
    menuLayout->setContentsMargins(4, 4, 4, 4);
    allAppsTab->setLayout(menuLayout);

    QLineEdit* menuSearchBox = new QLineEdit;
    menuSearchBox->setPlaceholderText("🔎 Search");  // u+01f50e - magnifier icon
    menuSearchBox->setClearButtonEnabled(true);
    allAppsTab->layout()->addWidget(menuSearchBox);

    QListWidget* menuAppsList = new QListWidget;
    allAppsTab->layout()->addWidget(menuAppsList);

    menuTabWidget->addTab(allAppsTab, "All applications");


    // UI: Favorite apps tab
    QWidget* favAppsTab = new QWidget;
    favAppsTab->setFont(font);

    QVBoxLayout* favLayout = new QVBoxLayout;
    favLayout->setContentsMargins(4, 4, 4, 4);
    favAppsTab->setLayout(favLayout);

    QListWidget* favAppsList = new QListWidget;
    favAppsTab->layout()->addWidget(favAppsList);

    //QPushButton* editFavListPushButton = new QPushButton;
    //editFavListPushButton->setText("Edit favorites");
    //favAppsTab->layout()->addWidget(editFavListPushButton);

    menuTabWidget->addTab(favAppsTab, "Favorites");


    // UI: Edit favorite apps widget
    /*QWidget* editFavsWidget = new QWidget;
    editFavsWidget->setBaseSize(350, 500);
    editFavsWidget->setWindowTitle("Edit favorites");

    QHBoxLayout* editLayout = new QHBoxLayout;
    editLayout->setContentsMargins(4, 4, 4, 4);
    editFavsWidget->setLayout(editLayout);

    QListWidget* avaliableApps = new QListWidget;
    editFavsWidget->layout()->addWidget(avaliableApps);
    QListWidget* addedApps = new QListWidget;
    editFavsWidget->layout()->addWidget(addedApps);

    QVBoxLayout* buttonsLayout = new QVBoxLayout;

    QPushButton* addSelected = new QPushButton;
    addSelected->setText(">>");
    buttonsLayout->addWidget(addSelected);

    QPushButton* removeSelected = new QPushButton;
    removeSelected->setText("<<");
    buttonsLayout->addWidget(removeSelected);

    QPushButton* moveUp = new QPushButton;
    moveUp->setText("↑");
    buttonsLayout->addWidget(moveUp);

    QPushButton* moveDown = new QPushButton;
    moveDown->setText("↓");
    buttonsLayout->addWidget(moveDown);

    editLayout->addLayout(buttonsLayout);*/


    // Make connections
    allAppsTab->connect(menuSearchBox, &QLineEdit::textEdited, allAppsTab,
                           [menuAppsList, menuSearchBox, this]() {
        buildMenu(menuAppsList, menuSearchBox->text());
    });

    allAppsTab->connect(menuAppsList, &QListWidget::itemDoubleClicked, allAppsTab,
                           [menuAppsList, appMenuWidget, this]() {
        execApp(execData[menuAppsList->selectedItems()[0]], appMenuWidget);
    });

    favAppsTab->connect(favAppsList, &QListWidget::itemDoubleClicked, favAppsTab,
                        [favAppsList, appMenuWidget, this]() {
        execApp(favExecData[favAppsList->selectedItems()[0]], appMenuWidget);
    });

    /*favAppsTab->connect(editFavListPushButton, &QPushButton::clicked, favAppsTab,
                        [editFavsWidget, avaliableApps, addedApps, appMenuWidget, this]() {
        if (editFavsWidget->isHidden()) {
            editFavsWidget->show();

            buildMenu(avaliableApps);
            buildFavMenu(addedApps, __favDesktopFiles__);

            appMenuWidget->hide();
        }
    });*/


    return {appMenuWidget, menuSearchBox, menuAppsList, favAppsList, menuTabWidget};
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


void AppMenu::buildFavMenu(QListWidget* favListWidget, QVariantList favDesktopFiles) {
    favListWidget->clear();
    favExecData.clear();
    if (favDesktopFiles != __favDesktopFiles__) {
        __favDesktopFiles__.clear();
    }

    QDir appDir("/usr/share/applications");
    QString pathToCurrentDesktopFile = "";
    App currentApp;

    for (int i = 0; i < favDesktopFiles.length(); ++i) {
        pathToCurrentDesktopFile = appDir.absoluteFilePath(favDesktopFiles[i].toString());
        currentApp = readDesktopFile(pathToCurrentDesktopFile);

        favListWidget->addItem(currentApp.displayedName);
        favListWidget->item(i)->setIcon(currentApp.icon);
        favExecData[favListWidget->item(i)] = currentApp.exec;

        if (favDesktopFiles != __favDesktopFiles__) {
            __favDesktopFiles__ << favDesktopFiles[i].toString();
        }
    }
}


void AppMenu::buildMenu(QListWidget* menuAppsList, QString searchRequest) {
    // TODO: ~/.local/share/applications
    QDir appDir("/usr/share/applications");
    QStringList desktopFilesList = appDir.entryList();

    QString pathToCurrentDesktopFile;
    App currentApp;

    desktopFilesList.removeFirst();
    desktopFilesList.removeFirst();

    menuAppsList->clear();
    execData.clear();

    for (int i = 0; i < desktopFilesList.length(); ++i) {
        pathToCurrentDesktopFile = appDir.absoluteFilePath(desktopFilesList[i]);
        currentApp = readDesktopFile(pathToCurrentDesktopFile);
        if (currentApp.display) {
            if (currentApp.displayedName != "") {
                if (currentApp.displayedName.contains(searchRequest, Qt::CaseInsensitive)) {
                    QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                    menuAppsList->addItem(item);
                    menuAppsList->setCurrentItem(item);
                    menuAppsList->currentItem()->setIcon(currentApp.icon);

                    execData[item] = currentApp.exec;
                }
            }
        }
    }

    menuAppsList->setCurrentRow(0);
}
