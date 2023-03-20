#include "appmenu.h"

void AppMenu::setAppletUI(QObject *execHolder,
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
                          double opacity) {
    this->setWindowTitle("App Menu");
    this->setObjectName("appMenu");

    // Window flags
    // HELP WANTED: Menu should not show up in Alt-Tab (do not use Qt::X11BypassWindowManagerHint)
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Geometry
    int width = 450, height = screenHeight / 2;
    int ax = 0, ay = 0;
    switch (panelLocation) {
        case Top:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = panelThickness + 5;
        break;

        case Bottom:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = screenHeight - panelThickness - height - 5;
        break;

        case Left:
            ax = panelThickness + 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;

        case Right:
            ax = screenWidth - panelThickness - width - 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;
    }
    this->setFixedSize(width, height);
    this->move(ax, ay);

    // Font
    this->setFont(font);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + stylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Opacity
    this->setWindowOpacity(opacity);


    // Main UI
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setContentsMargins(4, 4, 4, 4);

    mTabWidget = new QTabWidget();
    mTabWidget->setFont(font);
    mTabWidget->setTabShape((useTriangularTabs) ? QTabWidget::Triangular : QTabWidget::Rounded);
    mMainLayout->addWidget(mTabWidget);

    // All applications tab UI
    mAllAppsTab = new QWidget();
    mAllAppsLayout = new QVBoxLayout(mAllAppsTab);
    mAllAppsLayout->setContentsMargins(4, 4, 4, 4);

    mSearchBox = new QLineEdit();
    mSearchBox->setPlaceholderText("🔎 " + tr("Search"));  // u+01f50e - magnifier icon
    mSearchBox->setClearButtonEnabled(true);
    mAllAppsLayout->addWidget(mSearchBox);

    mAppsList = new QListWidget();
    mAppsList->setStyleSheet("QListView::item:selected { background-color: " + accent + "; color: #ffffff };");
    mAllAppsLayout->addWidget(mAppsList);

    mTabWidget->addTab(mAllAppsTab, tr("All applications"));

    // Favorite apps tab UI
    mFavAppsTab = new QWidget();
    mFavAppsLayout = new QVBoxLayout(mFavAppsTab);
    mFavAppsLayout->setContentsMargins(4, 4, 4, 4);

    mFavAppsList = new QListWidget();
    mFavAppsList->setStyleSheet("QListView::item:selected { background-color: " + accent + "; color: #ffffff };");
    mFavAppsLayout->addWidget(mFavAppsList);

    mTabWidget->addTab(mFavAppsTab, tr("Favorite apps"));

    // Run tab UI
    mRunTab = new QWidget();
    mRunLayout = new QVBoxLayout(mRunTab);
    mRunLayout->setContentsMargins(4, 4, 4, 4);

    mRunLabel = new QLabel(tr("Enter command:"));
    mRunLayout->addWidget(mRunLabel);

    mCmdLineEdit = new QLineEdit();
    mRunLayout->addWidget(mCmdLineEdit);

    mRunPushButton = new QPushButton(tr("Run"));
    mRunLayout->addWidget(mRunPushButton);

    mRunLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));

    mTabWidget->addTab(mRunTab, tr("Run"));


    // Make connections
    mAllAppsTab->connect(mSearchBox, &QLineEdit::textEdited, mAllAppsTab,
                         [this]() {
        buildMenu(mAppsList, mSearchBox->text());
    });
    mAllAppsTab->connect(mAppsList, &QListWidget::itemDoubleClicked, mAllAppsTab,
                         [this, execHolder]() {
        execApp(execHolder, mExecByItem[mAppsList->selectedItems()[0]]);
    });
    mFavAppsTab->connect(mFavAppsList, &QListWidget::itemDoubleClicked, mFavAppsTab,
                         [this, execHolder]() {
        execApp(execHolder, mExecByItem[mFavAppsList->selectedItems()[0]]);
    });
    mRunTab->connect(mRunPushButton, &QPushButton::clicked, this,
                     [this, execHolder]() {
        execApp(execHolder, mCmdLineEdit->text());
        mCmdLineEdit->clear();
    });
}

App AppMenu::readDesktopEntry(QString desktopEntryPath) {
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

void AppMenu::execApp(QObject *parent, QString exec) {
    QProcess *process = new QProcess(parent);
    // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html#exec-variables
    if (exec[exec.length()-2] == "%") {
        exec.chop(2);
    }
    process->start(exec);
    this->hide();
}

void AppMenu::buildMenu(QListWidget *appsList, QString filter) {
    QDir appDir("/usr/share/applications");
    QStringList desktopEntriesList = appDir.entryList();

    App currentApp;

    mExecByItem.clear();
    appsList->clear();

    for (int i = 2; i < desktopEntriesList.length(); ++i) {
        currentApp = readDesktopEntry(appDir.absoluteFilePath(desktopEntriesList[i]));
        if (currentApp.display) {
            if (currentApp.displayedName != "") {
                if (currentApp.displayedName.contains(filter, Qt::CaseInsensitive)) {
                    QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                    item->setIcon(currentApp.icon);
                    appsList->addItem(item);
                    mExecByItem[item] = currentApp.exec;
                }
            }
        }
    }

    QString homeDir = getenv("HOME");
    appDir.cd(homeDir + "/.local/share/applications");
    desktopEntriesList = appDir.entryList();

    for (int i = 2; i < desktopEntriesList.length(); ++i) {
        currentApp = readDesktopEntry(appDir.absoluteFilePath(desktopEntriesList[i]));
        if (currentApp.display) {
            if (currentApp.displayedName != "") {
                if (currentApp.displayedName.contains(filter, Qt::CaseInsensitive)) {
                    QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
                    item->setIcon(currentApp.icon);
                    appsList->addItem(item);
                    mExecByItem[item] = currentApp.exec;
                }
            }
        }
    }
}

void AppMenu::buildFavMenu(QListWidget *favAppsList, QVariantList *favDesktopEntries) {
    favAppsList->clear();

    QString homeDir = getenv("HOME");
    QDir globalAppDir("/usr/share/applications");
    QDir localAppDir(homeDir + "/.local/share/applications");

    QStringList globalDesktopEntriesList = globalAppDir.entryList();
    QStringList localDesktopEntriesList = localAppDir.entryList();
    QString desktopEntryPath;
    App currentApp;

    for (int i = 0; i < favDesktopEntries->length(); ++i) {
        QString favDesktopEntry = favDesktopEntries->at(i).toString();
        if (globalDesktopEntriesList.contains(favDesktopEntry)) {
            desktopEntryPath = globalAppDir.absoluteFilePath(favDesktopEntry);
            currentApp = readDesktopEntry(desktopEntryPath);
            QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
            item->setIcon(currentApp.icon);
            favAppsList->addItem(item);
        }
        else if (localDesktopEntriesList.contains(favDesktopEntry)) {
            desktopEntryPath = localAppDir.absoluteFilePath(favDesktopEntry);
            currentApp = readDesktopEntry(desktopEntryPath);
            QListWidgetItem* item = new QListWidgetItem(currentApp.displayedName);
            item->setIcon(currentApp.icon);
            favAppsList->addItem(item);
        }
    }
}

AppMenu::AppMenu(QObject *execHolder,
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
                 QVariantList *favApps) : QWidget(nullptr) {
    mFavApps = favApps;
    setAppletUI(execHolder,
                panelLocation,
                panelThickness,
                screenWidth,
                screenHeight,
                font,
                buttonCoord1,
                buttonCoord2,
                useTriangularTabs,
                accent,
                stylesheet,
                opacity);
    buildMenu(mAppsList, "");
    buildFavMenu(mFavAppsList, favApps);
}

AppMenu::~AppMenu() {
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
}
