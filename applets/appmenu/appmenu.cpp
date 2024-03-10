#include "appmenu.h"

void AppMenuApplet::externalWidgetSetup() {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(mCfgMan->mFont);
    mExternalWidget->setObjectName("appMenuButton");
    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);
    mExternalWidget->setToolTip("List of installed applications");
    int appMenuIconSize = mCfgMan->mMenuIconSize;
    static_cast<QPushButton*>(mExternalWidget)->setIconSize(
        QSize(appMenuIconSize, appMenuIconSize)
    );

    QString menuIcon = mCfgMan->mMenuIcon;
    if (QIcon::hasThemeIcon(menuIcon)) {
        static_cast<QPushButton*>(mExternalWidget)->setIcon(
            QIcon::fromTheme(menuIcon)
        );
    }
    else {
        static_cast<QPushButton*>(mExternalWidget)->setIcon(QIcon(menuIcon));
    }

    if (mParentPanel->mPanelLayout == Horizontal) {
        QString menuText = mCfgMan->mMenuText;
        if (!menuText.isEmpty()) {
            static_cast<QPushButton*>(mExternalWidget)->setText(
                QString(" %1").arg(menuText)
            );
        }
    }

    // Make connections
    connect(static_cast<QPushButton*>(mExternalWidget),
            &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            mTabWidget->setCurrentIndex(0);
            mSearchBox->clear();
            buildMenu(mAppsList, "");
            buildFavMenu(mCfgMan, mFavAppsList);
            if (mCfgMan->mTransparent) {
                setBlurredBackground();
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

void AppMenuApplet::internalWidgetSetup() {
    mInternalWidget = new QWidget();

    // Geometry
    QScreen* screen = mParentPanel->mPanelScreen;
    int width = 450, height = screen->geometry().height() / 2;
    preliminaryInternalWidgetSetup(width, height, true);

    mInternalWidget->setObjectName("appMenu");
    mInternalWidget->setWindowTitle("plainDE App Menu");

    // Main UI
    mMainLayout = new QVBoxLayout(mInternalWidget);
    mMainLayout->setContentsMargins(4, 4, 4, 4);

    mTabWidget = new QTabWidget();
    mTabWidget->setFont(mCfgMan->mFont);
    mTabWidget->setTabShape((mCfgMan->mUseTriangularTabs) ? QTabWidget::Triangular : QTabWidget::Rounded);
    mMainLayout->addWidget(mTabWidget);

    // All applications tab UI
    mAllAppsTab = new QWidget();

    mAllAppsLayout = new QVBoxLayout(mAllAppsTab);
    mAllAppsLayout->setContentsMargins(4, 4, 4, 4);

    mSearchBox = new QLineEdit();
    mSearchBox->setPlaceholderText("🔎 " + tr("Search"));  // u+01f50e - magnifier icon
    mSearchBox->setClearButtonEnabled(true);
    mSearchBox->setFont(mCfgMan->mFont);
    mAllAppsLayout->addWidget(mSearchBox);

    mAppsList = new QListWidget();
    mAppsList->setFont(mCfgMan->mFont);
    mAppsList->setStyleSheet("QListView::item:selected { background-color: " + mCfgMan->mAccent + "; color: #ffffff };");
    mAllAppsLayout->addWidget(mAppsList);

    mTabWidget->addTab(mAllAppsTab, tr("All applications"));

    // Favorite apps tab UI
    mFavAppsTab = new QWidget();
    mFavAppsLayout = new QVBoxLayout(mFavAppsTab);
    mFavAppsLayout->setContentsMargins(4, 4, 4, 4);

    mFavAppsList = new QListWidget();
    mFavAppsList->setFont(mCfgMan->mFont);
    mFavAppsList->setStyleSheet("QListView::item:selected { background-color: " + mCfgMan->mAccent + "; color: #ffffff };");
    mFavAppsLayout->addWidget(mFavAppsList);

    mTabWidget->addTab(mFavAppsTab, tr("Favorite apps"));

    // Run tab UI
    mRunTab = new QWidget();
    mRunLayout = new QVBoxLayout(mRunTab);
    mRunLayout->setContentsMargins(4, 4, 4, 4);

    mRunLabel = new QLabel(tr("Enter command:"));
    mRunLabel->setFont(mCfgMan->mFont);
    mRunLayout->addWidget(mRunLabel);

    mCmdLineEdit = new QLineEdit();
    mCmdLineEdit->setFont(mCfgMan->mFont);
    mRunLayout->addWidget(mCmdLineEdit);

    mRunPushButton = new QPushButton(tr("Run"));
    mRunPushButton->setFont(mCfgMan->mFont);
    mRunLayout->addWidget(mRunPushButton);

    mRunLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));

    mTabWidget->addTab(mRunTab, tr("Run"));

    // Build menus
    buildMenu(mAppsList, "");
    buildFavMenu(mCfgMan, mFavAppsList);

    // Make connections
    mAllAppsTab->connect(mSearchBox, &QLineEdit::textEdited, mAllAppsTab,
                         [this]() {
        buildMenu(mAppsList, mSearchBox->text());
    });
    mAllAppsTab->connect(mAppsList, &QListWidget::itemActivated, mAllAppsTab,
                         [this]() {
        execApp(mExecByItem[mAppsList->selectedItems()[0]]);
    });
    mFavAppsTab->connect(mFavAppsList, &QListWidget::itemActivated, mFavAppsTab,
                         [this]() {
        execApp(mExecByItem[mFavAppsList->selectedItems()[0]]);
    });
    mRunTab->connect(mRunPushButton, &QPushButton::clicked, mInternalWidget,
                     [this]() {
        if (!mCmdLineEdit->text().isEmpty()) {
            execApp(mCmdLineEdit->text());
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

void AppMenuApplet::execApp(QString exec) {
    QObject* parent = mParentPanel->mExecHolder;
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
                             Panel* parentPanel) : StaticApplet(
                                                       "org.plainDE.appMenu",
                                                       cfgMan,
                                                       parentPanel
                                                   ) {

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
