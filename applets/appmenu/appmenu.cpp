#include "appmenu.h"

void AppMenuApplet::externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel) {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(cfgMan->mFont);
    mExternalWidget->setObjectName("appMenuButton");
    mExternalWidget->setFlat(true);
    mExternalWidget->setToolTip("List of installed applications");
    int appMenuIconSize = cfgMan->mMenuIconSize;
    mExternalWidget->setIconSize(QSize(appMenuIconSize, appMenuIconSize));

    QString menuIcon = cfgMan->mMenuIcon;
    if (QIcon::hasThemeIcon(menuIcon)) {
        mExternalWidget->setIcon(QIcon::fromTheme(menuIcon));
    }
    else {
        mExternalWidget->setIcon(QIcon(menuIcon));
    }

    if (parentPanel->mPanelLayout == Horizontal) {
        QString menuText = cfgMan->mMenuText;
        if (!menuText.isEmpty()) {
            mExternalWidget->setText(QString(" %1").arg(menuText));
        }
    }

    // Make connections
    connect(mExternalWidget, &QPushButton::clicked, this, [this, cfgMan]() {
        if (!mInternalWidget->isVisible()) {
            mTabWidget->setCurrentIndex(0);
            mSearchBox->clear();
            buildMenu(mAppsList, "");
            buildFavMenu(cfgMan, mFavAppsList);
            if (cfgMan->mTransparent) {
                setBlurredBackground(mInternalWidget);
            }
            mInternalWidget->show();
        }
        else {
            mAppsList->clear();
            mFavAppsList->clear();
            mInternalWidget->hide();
        }
    });
}

void AppMenuApplet::internalWidgetSetup(ConfigManager* cfgMan,
                                        Panel* parentPanel) {
    mInternalWidget = new QWidget();

    // Geometry
    QScreen* screen = parentPanel->mPanelScreen;
    int width = 450, height = screen->geometry().height() / 2;
    preliminaryInternalWidgetSetup(mInternalWidget,
                                   mExternalWidget,
                                   cfgMan,
                                   parentPanel,
                                   width,
                                   height,
                                   true);

    mInternalWidget->setObjectName("appMenu");
    mInternalWidget->setWindowTitle("plainDE App Menu");

    // Main UI
    mMainLayout = new QVBoxLayout(mInternalWidget);
    mMainLayout->setContentsMargins(4, 4, 4, 4);

    mTabWidget = new QTabWidget();
    mTabWidget->setFont(cfgMan->mFont);
    mTabWidget->setTabShape((cfgMan->mUseTriangularTabs) ? QTabWidget::Triangular : QTabWidget::Rounded);
    mMainLayout->addWidget(mTabWidget);

    // All applications tab UI
    mAllAppsTab = new QWidget();

    mAllAppsLayout = new QVBoxLayout(mAllAppsTab);
    mAllAppsLayout->setContentsMargins(4, 4, 4, 4);

    mSearchBox = new QLineEdit();
    mSearchBox->setPlaceholderText("🔎 " + tr("Search"));  // u+01f50e - magnifier icon
    mSearchBox->setClearButtonEnabled(true);
    mSearchBox->setFont(cfgMan->mFont);
    mAllAppsLayout->addWidget(mSearchBox);

    mAppsList = new QListWidget();
    mAppsList->setFont(cfgMan->mFont);
    mAppsList->setStyleSheet("QListView::item:selected { background-color: " + cfgMan->mAccent + "; color: #ffffff };");
    mAllAppsLayout->addWidget(mAppsList);

    mTabWidget->addTab(mAllAppsTab, tr("All applications"));

    // Favorite apps tab UI
    mFavAppsTab = new QWidget();
    mFavAppsLayout = new QVBoxLayout(mFavAppsTab);
    mFavAppsLayout->setContentsMargins(4, 4, 4, 4);

    mFavAppsList = new QListWidget();
    mFavAppsList->setFont(cfgMan->mFont);
    mFavAppsList->setStyleSheet("QListView::item:selected { background-color: " + cfgMan->mAccent + "; color: #ffffff };");
    mFavAppsLayout->addWidget(mFavAppsList);

    mTabWidget->addTab(mFavAppsTab, tr("Favorite apps"));

    // Run tab UI
    mRunTab = new QWidget();
    mRunLayout = new QVBoxLayout(mRunTab);
    mRunLayout->setContentsMargins(4, 4, 4, 4);

    mRunLabel = new QLabel(tr("Enter command:"));
    mRunLabel->setFont(cfgMan->mFont);
    mRunLayout->addWidget(mRunLabel);

    mCmdLineEdit = new QLineEdit();
    mCmdLineEdit->setFont(cfgMan->mFont);
    mRunLayout->addWidget(mCmdLineEdit);

    mRunPushButton = new QPushButton(tr("Run"));
    mRunPushButton->setFont(cfgMan->mFont);
    mRunLayout->addWidget(mRunPushButton);

    mRunLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));

    mTabWidget->addTab(mRunTab, tr("Run"));

    // Build menus
    buildMenu(mAppsList, "");
    buildFavMenu(cfgMan, mFavAppsList);

    // Make connections
    mAllAppsTab->connect(mSearchBox, &QLineEdit::textEdited, mAllAppsTab,
                         [this]() {
        buildMenu(mAppsList, mSearchBox->text());
    });
    mAllAppsTab->connect(mAppsList, &QListWidget::itemActivated, mAllAppsTab,
                         [this, parentPanel]() {
        execApp(parentPanel->mExecHolder, mExecByItem[mAppsList->selectedItems()[0]]);
    });
    mFavAppsTab->connect(mFavAppsList, &QListWidget::itemActivated, mFavAppsTab,
                         [this, parentPanel]() {
        execApp(parentPanel->mExecHolder, mExecByItem[mFavAppsList->selectedItems()[0]]);
    });
    mRunTab->connect(mRunPushButton, &QPushButton::clicked, mInternalWidget,
                     [this, parentPanel]() {
        if (!mCmdLineEdit->text().isEmpty()) {
            execApp(parentPanel->mExecHolder, mCmdLineEdit->text());
            mCmdLineEdit->clear();
        }
    });
}

App AppMenuApplet::readDesktopEntry(QString desktopEntryPath) {
    App app;
    QString iconPath;

    QSettings desktopFileReader(desktopEntryPath, QSettings::IniFormat);
    desktopFileReader.sync();
    desktopFileReader.beginGroup("Desktop Entry");
        if (!desktopFileReader.value("NoDisplay").toBool()) {
            app.display = true;
            app.displayedName = desktopFileReader.value("Name").toString();
            app.exec = desktopFileReader.value("Exec").toString();
            iconPath = desktopFileReader.value("Icon").toString();
            if (QIcon::hasThemeIcon(iconPath)) {
                app.icon = QIcon::fromTheme(iconPath);
            }
            else {
                if (QFile::exists(iconPath)) {
                    app.icon = QIcon(iconPath);
                }
                else {
                    app.icon = QIcon::fromTheme("dialog-question");
                }
            }
        }
        else {
            app.display = false;
        }
    desktopFileReader.endGroup();

    return app;
}

void AppMenuApplet::execApp(QObject* parent, QString exec) {
    QProcess* process = new QProcess(parent);
    // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html#exec-variables
    if (exec[exec.length()-2] == "%") {
        exec.chop(2);
    }
    process->start(exec);
    mInternalWidget->hide();
}

void AppMenuApplet::buildMenu(QListWidget* appsList, QString filter) {
    QDir appDir("/usr/share/applications");
    QStringList desktopEntriesList = appDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    App currentApp;

    mExecByItem.clear();
    appsList->clear();

    for (int i = 0; i < desktopEntriesList.length(); ++i) {
        currentApp = readDesktopEntry(appDir.absoluteFilePath(desktopEntriesList[i]));
        if (desktopEntriesList[i].endsWith(".desktop")) {
            if (currentApp.display) {
                if (currentApp.displayedName.contains(filter, Qt::CaseInsensitive)) {
                    QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                    if (!currentApp.icon.isNull()) {
                        item->setIcon(currentApp.icon);
                    }
                    else {
                        item->setIcon(QIcon::fromTheme("dialog-question"));
                    }
                    appsList->addItem(item);
                    mExecByItem[item] = currentApp.exec;
                }
            }
        }
    }

    QString homeDir = getenv("HOME");
    if (QDir(homeDir + "/.local/share/applications").exists()) {
        appDir.cd(homeDir + "/.local/share/applications");
        desktopEntriesList = appDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

        for (int i = 0; i < desktopEntriesList.length(); ++i) {
            currentApp = readDesktopEntry(appDir.absoluteFilePath(desktopEntriesList[i]));
            if (desktopEntriesList[i].endsWith(".desktop")) {
                if (currentApp.display) {
                    if (currentApp.displayedName.contains(filter, Qt::CaseInsensitive)) {
                        QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                        if (!currentApp.icon.isNull()) {
                            item->setIcon(currentApp.icon);
                        }
                        else {
                            item->setIcon(QIcon::fromTheme("dialog-question"));
                        }
                        appsList->addItem(item);
                        mExecByItem[item] = currentApp.exec;
                    }
                }
            }
        }
    }

    appsList->sortItems();
}

void AppMenuApplet::buildFavMenu(ConfigManager* cfgMan, QListWidget* favAppsList) {
    favAppsList->clear();

    QString homeDir = getenv("HOME");
    QDir globalAppDir("/usr/share/applications");
    QDir localAppDir(homeDir + "/.local/share/applications");

    QStringList globalDesktopEntriesList = globalAppDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList localDesktopEntriesList = localAppDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QString desktopEntryPath;
    App currentApp;

    for (int i = 0; i < cfgMan->mFavApps.count(); ++i) {
        QString favDesktopEntry = cfgMan->mFavApps.at(i).toString();
        if (globalDesktopEntriesList.contains(favDesktopEntry)) {
            desktopEntryPath = globalAppDir.absoluteFilePath(favDesktopEntry);
            currentApp = readDesktopEntry(desktopEntryPath);
            QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
            if (!currentApp.icon.isNull()) {
                item->setIcon(currentApp.icon);
            }
            else {
                item->setIcon(QIcon::fromTheme("dialog-question"));
            }
            favAppsList->addItem(item);
            mExecByItem[item] = currentApp.exec;
        }
        else if (localDesktopEntriesList.contains(favDesktopEntry)) {
            desktopEntryPath = localAppDir.absoluteFilePath(favDesktopEntry);
            currentApp = readDesktopEntry(desktopEntryPath);
            QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
            if (!currentApp.icon.isNull()) {
                item->setIcon(currentApp.icon);
            }
            else {
                item->setIcon(QIcon::fromTheme("dialog-question"));
            }
            favAppsList->addItem(item);
            mExecByItem[item] = currentApp.exec;
        }
    }

    favAppsList->sortItems();
}

AppMenuApplet::AppMenuApplet(ConfigManager* cfgMan,
                             Panel* parentPanel,
                             QString additionalInfo) : Applet(cfgMan, parentPanel, additionalInfo) {

}

AppMenuApplet::~AppMenuApplet() {
    mInternalWidget->hide();

    while (mAppsList->count() > 0) {
        delete mAppsList->item(0);
    }
    while (mFavAppsList->count() > 0) {
        delete mFavAppsList->item(0);
    }

    mExecByItem.clear();

    delete mRunPushButton;
    delete mCmdLineEdit;
    delete mRunLabel;
    delete mRunLayout;
    delete mRunTab;

    delete mFavAppsList;
    delete mFavAppsLayout;
    delete mFavAppsTab;

    delete mAppsList;
    delete mSearchBox;
    delete mAllAppsLayout;
    delete mAllAppsTab;

    delete mTabWidget;
    delete mMainLayout;

    delete mInternalWidget;
    delete mExternalWidget;
}
