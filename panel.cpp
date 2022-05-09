#include "panel.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QMap>
#include <QList>
#include <QPropertyAnimation>

#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QDial>
#include <QHBoxLayout>
#include <QSpacerItem>

#include <KWindowSystem>

#include "applet.h"
#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"
#include "applets/kblayout/kblayout.h"
#include "applets/usermenu/usermenu.h"
#include "applets/volume/volume.h"
#include "applets/windowlist/windowlist.h"

Display* display;
QJsonObject config;
QMap<QString,QWidget*> appletWidgets;
PanelLocation location;
QFont panelFont;
WId panelWinID;
QScreen* primaryScreen;

QVariantList activatedAppletsList;

AppMenu* menu;
menuUI _menuUI;

QHBoxLayout* windowListLayout = new QHBoxLayout;
QList<WId> winIDs;
QHash<WId,QPushButton*> winWidgets;
unsigned short minWidth;

DateTimeApplet _dateTime;
dateTimeUI _dateTimeUI;

KbLayoutApplet _kbLayout;

UserMenuApplet _userMenu;
userMenuUI _userMenuUI;

TrayApplet _tray;
QStringList* trayEntries;

qint8 visibleDesktop;
qint8 countWorkspaces;

QString accent;


void readConfig() {
    // set globally readable variable for reading settings

    QString homeDirectory = getenv("HOME");
    QFile file;
    QString data;

    if (!QFile::exists(homeDirectory + "/.config/plainDE/config.json")) {
        qDebug() << homeDirectory + "/.config/plainDE/config.json" + " does not exist. Generating new...";
        system("python3 /usr/share/plainDE/genconfig.py");
    }

    file.setFileName(homeDirectory + "/.config/plainDE/config.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data = file.readAll();
    file.close();
    config = QJsonDocument::fromJson(data.toUtf8()).object();
}

void basicInit(panel* w) {
    if (QString::compare(getenv("XDG_SESSION_TYPE"), "x11") != 0) {
        qDebug() << "plainPanel currently works only on X11. Quitting...";
        QApplication::quit();
    }

    w->setWindowTitle("plainPanel");
    w->setObjectName("panel");

    QDir::setCurrent(getenv("HOME"));

    display = XOpenDisplay(getenv("DISPLAY"));

    panelWinID = w->winId();

    visibleDesktop = KWindowSystem::currentDesktop();
    countWorkspaces = KWindowSystem::numberOfDesktops();

    primaryScreen = QGuiApplication::primaryScreen();
}

void setPanelGeometry(panel* w) {
    // size, location, monitor settings, window flags

    // Get screens
    // QList<QScreen*> screensList = QGuiApplication::screens();
    // QGuiApplication::primaryScreen()->name();

    // Size & location
    unsigned short ax = 0, ay = 0;
    unsigned short panelWidth = primaryScreen->size().width();
    unsigned short panelHeight = config["panelHeight"].toInt();
    qint8 topStrut = panelHeight, bottomStrut = 0;

    if (config["panelLocation"].toString() == "bottom") {
        ay = primaryScreen->size().height() - panelHeight;
        topStrut = 0;
        bottomStrut = panelHeight;
    }

    w->setFixedHeight(panelHeight);
    w->setMaximumWidth(panelWidth);
    if (config["expandPanel"].toBool()) {
        w->setFixedWidth(panelWidth);
    }
    else {
        ax = config["xOffset"].toInt();
    }

    w->move(ax, ay);

    // _NET_WM_STRUT - Bugfix #4
    KWindowSystem::setStrut(panelWinID, 0, 0, topStrut, bottomStrut);

    // Moving panel on other workspaces - Bugfix #3
    w->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, w, []() {
        KWindowSystem::setOnDesktop(panelWinID, KWindowSystem::currentDesktop());
    });

    // Flags
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}

void panel::updateGeometry() {
    unsigned short panelWidth = primaryScreen->size().width();
    unsigned short panelHeight = config["panelHeight"].toInt();

    this->setFixedHeight(panelHeight);
    this->setGeometry(this->geometry().x(),
                      this->geometry().y(),
                      minWidth,
                      panelHeight);
    if (config["expandPanel"].toBool()) {
        this->setFixedWidth(panelWidth);
    }
}

void setRepeatingActions(panel* w) {
    // here we bring to life main QTimer for updating applets data

    QTimer* updateAppletsDataTimer = new QTimer(w);  // we should find a better way to update applets
    updateAppletsDataTimer->setInterval(200);
    w->connect(updateAppletsDataTimer, &QTimer::timeout, w, &panel::updateAppletsData);
    updateAppletsDataTimer->start();
}

void setPanelUI(panel* w) {
    // Set font
    panelFont.setFamily(config["fontFamily"].toString());
    panelFont.setPointSize(config["fontSize"].toInt());
    w->setFont(panelFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + config["theme"].toString());
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    w->setStyleSheet(styleSheet.readAll());

    // Set accent
    accent = config["accent"].toString();

    // Icons
    QIcon::setThemeName(config["iconTheme"].toString());

    // Layout
    QHBoxLayout* uiLayout = new QHBoxLayout;
    uiLayout->setContentsMargins(1, 1, 1, 1);
    w->setLayout(uiLayout);

    // Applets: show applets
    QFontMetrics fm(panelFont);
    location = (config["panelLocation"].toString() == "top") ? top : bottom;

    /* We could use QVariantList::contains, but this approach will not observe
     * order of placing applets, using loop.*/

    QVariantList activeAppletsList = config["applets"].toArray().toVariantList();
    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            w->layout()->addWidget(appMenuPushButton);
            appletWidgets["appMenuPushButton"] = appMenuPushButton;

            if (QIcon::hasThemeIcon(config["menuIcon"].toString())) {
                appMenuPushButton->setIcon(QIcon::fromTheme(config["menuIcon"].toString()));
            }
            else {
                appMenuPushButton->setIcon(QIcon(config["menuIcon"].toString()));
            }

            appMenuPushButton->setText(" " + config["menuText"].toString());
        }

        else if (applet == "spacer") {
            w->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
        }

        else if (applet == "splitter") {
            QLabel* splitter = new QLabel("|");
            w->layout()->addWidget(splitter);
        }

        else if (applet == "datetime") {
            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            w->layout()->addWidget(dateTimePushButton);

            appletWidgets["dateTimePushButton"] = dateTimePushButton;

            if (config["showDate"].toBool()) {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }
        }

        else if (applet == "kblayout") {
            QPushButton* layoutName = new QPushButton;
            appletWidgets["layoutName"] = layoutName;

            short buttonWidth = fm.horizontalAdvance("AA");
            layoutName->setMaximumWidth(buttonWidth);
            layoutName->setFlat(true);

            w->layout()->addWidget(layoutName);
        }

        else if (applet == "usermenu") {
            QPushButton* userMenuPushButton = new QPushButton;
            userMenuPushButton->setFlat(true);
            userMenuPushButton->setText(getenv("USER"));
            appletWidgets["userMenuPushButton"] = userMenuPushButton;
            userMenuPushButton->setObjectName("userMenuPushButton");

            short userMenuWidth = fm.horizontalAdvance(getenv("USER")) + 20;
            userMenuPushButton->setMaximumWidth(userMenuWidth);

            userMenuPushButton->setIcon(QIcon::fromTheme("computer"));

            w->layout()->addWidget(userMenuPushButton);
        }

        else if (applet == "volume") {
            QDial* volumeDial = new QDial;
            volumeDial->setMinimum(0);
            volumeDial->setMaximum(100);
            volumeDial->setValue(50);
            volumeDial->setMaximumWidth(25);

            QLabel* volumeLabel = new QLabel("50%");
            volumeLabel->setMaximumWidth(fm.horizontalAdvance("100%"));

            w->layout()->addWidget(volumeDial);
            w->layout()->addWidget(volumeLabel);

            VolumeApplet::setVolume(50);

            appletWidgets["volumeDial"] = volumeDial;
            appletWidgets["volumeLabel"] = volumeLabel;
        }

        else if (applet == "windowlist") {
            uiLayout->addLayout(windowListLayout);
        }

        else if (applet == "workspaces") {
            qint8 countWorkspaces = KWindowSystem::numberOfDesktops();
            for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
                QPushButton* currentWorkspace = new QPushButton(QString::number(workspace+1));
                currentWorkspace->setMaximumWidth(fm.horizontalAdvance("100"));
                currentWorkspace->setStyleSheet("background-color: #9a9996; color: #000000;");
                appletWidgets["workspace" + QString::number(workspace+1)] = currentWorkspace;

                if (KWindowSystem::currentDesktop() == workspace+1) {
                    currentWorkspace->setStyleSheet("background-color: " + accent + "; color: #ffffff;");
                }
                w->layout()->addWidget(currentWorkspace);
            }
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }

    w->show();

    // Applets: set actions
    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            _menuUI = menu->__createUI__(location, config["panelHeight"].toInt(),
                                        panelFont, appletWidgets["appMenuPushButton"]->x(),
                                        appletWidgets["appMenuPushButton"]->geometry().topRight().x(),
                                        config["appMenuTriangularTabs"].toBool(), accent,
                                        config["theme"].toString());
            w->connect(static_cast<QPushButton*>(appletWidgets["appMenuPushButton"]), &QPushButton::clicked, w, &panel::toggleAppMenu);
        }

        else if (applet == "datetime") {
            Qt::DayOfWeek day;
            QString configDay = config["firstDayOfWeek"].toString();
            // temp
            if (configDay == "Monday") {
                day = Qt::Monday;
            }
            else if (configDay == "Tuesday") {
                day = Qt::Tuesday;
            }
            else if (configDay == "Wednesday") {
                day = Qt::Wednesday;
            }
            else if (configDay == "Thursday") {
                day = Qt::Thursday;
            }
            else if (configDay == "Friday") {
                day = Qt::Friday;
            }
            else if (configDay == "Saturday") {
                day = Qt::Saturday;
            }
            else {
                day = Qt::Sunday;
            }

            _dateTimeUI = _dateTime.__createUI__(location,
                                                 config["panelHeight"].toInt(),
                                                 panelFont,
                                                 appletWidgets["dateTimePushButton"]->pos().x(),
                                                 appletWidgets["dateTimePushButton"]->geometry().topRight().x(),
                                                 day);

            w->connect(static_cast<QPushButton*>(appletWidgets["dateTimePushButton"]),
                       &QPushButton::clicked,
                       w,
                       &panel::toggleCalendar);
        }

        else if (applet == "kblayout") {
            _kbLayout.__init__();
            static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
        }

        else if (applet == "usermenu") {
            _userMenuUI = _userMenu.__createUI__(location,
                                          config["panelHeight"].toInt(),
                                          panelFont,
                                          appletWidgets["userMenuPushButton"]->pos().x(),
                                          appletWidgets["userMenuPushButton"]->geometry().topRight().x(),
                                          config["theme"].toString());

            w->connect(static_cast<QPushButton*>(appletWidgets["userMenuPushButton"]),
                       &QPushButton::clicked,
                       w,
                       &panel::toggleUserMenu);
        }

        else if (applet == "volume") {
            w->connect(static_cast<QDial*>(appletWidgets["volumeDial"]),
                       &QDial::valueChanged,
                       w,
                       &panel::setVolume);
        }

        else if (applet == "windowlist") {
            WindowList wl;
            QList<WId> wins = wl.getWinList();

            for (auto it = wins.cbegin(), end = wins.cend(); it != end; ++it) {
                if (*it != panelWinID) {
                    KWindowInfo nameInfo(*it, NET::WMName);
                    QPixmap icon = KWindowSystem::icon(*it, -1, config["panelHeight"].toInt(), true);
                    QString winName = nameInfo.name();
                    short size = winName.size();
                    winName.truncate(15);
                    if (size > 15) {
                        winName += "...";
                    }
                    QPushButton* windowButton = new QPushButton(winName);
                    winWidgets[*it] = windowButton;
                    windowListLayout->addWidget(windowButton);
                    windowButton->setIcon(icon);

                    w->connect(windowButton, &QPushButton::clicked, w, [windowButton]() {
                        KWindowInfo windowInfo(winWidgets.key(windowButton), NET::WMState | NET::XAWMState);
                        if (windowInfo.mappingState() != NET::Visible || windowInfo.isMinimized()) {
                            KWindowSystem::unminimizeWindow(winWidgets.key(windowButton));
                        }
                        else {
                            KWindowSystem::minimizeWindow(winWidgets.key(windowButton));
                        }
                    });

                    w->connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, w, []() {
                        WindowList wl;
                        QList<unsigned long long> keys = winWidgets.keys();
                        winIDs.clear();
                        winIDs = wl.getWinList();

                        foreach (unsigned long long id, keys) {
                            if (!winIDs.contains(id)) {
                                delete winWidgets[id];
                                winWidgets.remove(id);
                            }
                        }
                    });
                }
            }
        }

        else if (applet == "workspaces") {
            for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
                w->connect(static_cast<QPushButton*>(appletWidgets["workspace" + QString::number(workspace+1)]),
                        &QPushButton::clicked, w, [workspace]() {
                    KWindowSystem::setCurrentDesktop(workspace+1);
                });
            }
        }
    }

    activatedAppletsList = config["applets"].toArray().toVariantList();
    minWidth = w->width();

    setRepeatingActions(w);
}


// Slots
void panel::toggleAppMenu() {
    if (!_menuUI.menuWidget->isVisible()) {
        _menuUI.searchBox->clear();
        menu->buildMenu(_menuUI.appListWidget, "");
        menu->buildFavMenu(_menuUI.favListWidget, config["favApps"].toArray().toVariantList());
        _menuUI.tabWidget->setCurrentIndex(0);

        short ax;
        short offset = (config["expandPanel"].toBool() ? 0 : config["xOffset"].toInt());
        short buttonX = appletWidgets["appMenuPushButton"]->x();
        short buttonXRight = appletWidgets["appMenuPushButton"]->geometry().topRight().x();
        short appMenuWidth = 450;

        if (primaryScreen->geometry().width() - (buttonX + offset) >= appMenuWidth) {
            ax = buttonX + offset;
        }
        else {
            ax = (buttonXRight + offset) - appMenuWidth;
        }

        _menuUI.menuWidget->move(ax,
                                 _menuUI.menuWidget->geometry().y());
        _menuUI.menuWidget->show();

    }
    else {
        _menuUI.menuWidget->hide();
    }
}

void panel::toggleCalendar() {
    if (!_dateTimeUI.calendarWidget->isVisible()) {
        short ax;
        short offset = (config["expandPanel"].toBool() ? 0 : config["xOffset"].toInt());
        short buttonX = appletWidgets["dateTimePushButton"]->x();
        short buttonXRight = appletWidgets["dateTimePushButton"]->geometry().topRight().x();
        short calendarWidth = 300;
        if (primaryScreen->geometry().width() - (buttonX + offset) >= calendarWidth) {
            ax = buttonX + offset;
        }
        else {
            ax = (buttonXRight + offset) - calendarWidth;
        }

        _dateTimeUI.calendarWidget->move(ax,
                                         _dateTimeUI.calendarWidget->geometry().y());
        _dateTimeUI.calendarWidget->show();
    }
    else _dateTimeUI.calendarWidget->hide();
}

void panel::toggleUserMenu() {
    if (!_userMenuUI.userMenuWidget->isVisible()) {
        QFontMetrics fm(panelFont);
        short userMenuWidth = fm.horizontalAdvance("About plainDE") + 20;
        short ax;
        short offset = (config["expandPanel"].toBool() ? 0 : config["xOffset"].toInt());
        short buttonX = appletWidgets["userMenuPushButton"]->x();
        short buttonXRight = appletWidgets["userMenuPushButton"]->geometry().topRight().x();

        if (primaryScreen->geometry().width() - (buttonX + offset) >= userMenuWidth) {
            ax = buttonX + offset;
        }
        else {
            ax = (buttonXRight + offset) - userMenuWidth;
        }

        _userMenuUI.userMenuWidget->move(ax,
                                         _userMenuUI.userMenuWidget->geometry().y());
        _userMenuUI.userMenuWidget->show();
    }
    else _userMenuUI.userMenuWidget->hide();
}

void panel::filterAppsList() {
    menu->buildMenu(_menuUI.appListWidget, _menuUI.searchBox->text());
}

void panel::setVolume() {
    short newValue = static_cast<QDial*>(appletWidgets["volumeDial"])->value();
    VolumeApplet::setVolume(newValue);
    static_cast<QLabel*>(appletWidgets["volumeLabel"])->setText(QString::number(newValue) + "%");
}

void panel::updateWindowList() {
    WindowList wl;
    winIDs.clear();
    winIDs = wl.getWinList();

    for (auto it = winIDs.cbegin(), end = winIDs.cend(); it != end; ++it) {
        if (*it != panelWinID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!winWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, config["panelHeight"].toInt(), true);
                QString winName = nameInfo.name();
                short size = winName.size();
                winName.truncate(15);
                if (size > 15) {
                    winName += "...";
                }
                QPushButton* windowButton = new QPushButton(winName);
                windowButton->setIcon(icon);
                winWidgets[*it] = windowButton;
                windowListLayout->addWidget(windowButton);

                this->connect(windowButton, &QPushButton::clicked, this, [windowButton]() {
                    KWindowInfo windowInfo(winWidgets.key(windowButton), NET::WMState | NET::XAWMState);
                    if (windowInfo.mappingState() != NET::Visible || windowInfo.isMinimized()) {
                        KWindowSystem::unminimizeWindow(winWidgets.key(windowButton));
                    }
                    else {
                        KWindowSystem::minimizeWindow(winWidgets.key(windowButton));
                    }
                });

                this->connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, []() {
                    WindowList wl;
                    QList<unsigned long long> keys = winWidgets.keys();
                    winIDs.clear();
                    winIDs = wl.getWinList();

                    foreach (unsigned long long id, keys) {
                        if (!winIDs.contains(id)) {
                            delete winWidgets[id];
                            winWidgets.remove(id);
                        }
                    }
                });
            }
            else {
                if (desktopInfo.isOnCurrentDesktop()) {
                    KWindowInfo nameInfo(*it, NET::WMName);
                    QString newName = nameInfo.name();
                    short sz = newName.length();
                    newName.truncate(15);
                    if (newName.length() == sz) {
                        if (winWidgets[*it]->text() != newName) {
                            winWidgets[*it]->setText(newName);
                        }
                    }
                    else {
                        if (winWidgets[*it]->text() != newName + "...") {
                            winWidgets[*it]->setText(newName + "...");
                        }
                    }
                }
                else {
                    delete winWidgets[*it];
                    winWidgets.remove(*it);
                }
            }
        }
    }
}

void panel::updateWorkspaces() {
    if (KWindowSystem::currentDesktop() != visibleDesktop) {
        visibleDesktop = KWindowSystem::currentDesktop();
        for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
            if ((workspace+1) == visibleDesktop) {
                appletWidgets["workspace" + QString::number(workspace+1)]->setStyleSheet(
                            "background-color: " + accent + "; color: #ffffff;");
            }
            else {
                appletWidgets["workspace" + QString::number(workspace+1)]->setStyleSheet(
                            "background-color: #9a9996; color: #000000;");
            }
        }
    }
}

void panel::updateAppletsData() {
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "datetime") {
            if (config["showDate"].toBool()) {
               static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }
        }

        else if (applet == "kblayout") {
            static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
        }

        else if (applet == "windowlist") {
            updateWindowList();
            if (!config["expandPanel"].toBool()) {
                updateGeometry();
            }
        }
        else if (applet == "workspaces") {
            updateWorkspaces();
        }
    }
}

void panel::animation(panel* w) {
    if (config["enableAnimation"].toBool()) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(w, "pos");
        panelAnimation->setDuration(250);

        QScreen* primaryScreen = QGuiApplication::primaryScreen();

        unsigned short panelHeight = config["panelHeight"].toInt();
        unsigned short ax = (config["expandPanel"].toBool() ? 0 : config["xOffset"].toInt());

        if (config["panelLocation"].toString() == "top") {
            panelAnimation->setStartValue(QPoint(ax, -panelHeight));
            panelAnimation->setEndValue(QPoint(ax, 0));
        }
        else {
            panelAnimation->setStartValue(QPoint(ax, primaryScreen->size().height()));
            panelAnimation->setEndValue(QPoint(ax, primaryScreen->size().height() - panelHeight));
        }
        panelAnimation->start();
    }
}

void autostart(panel* w) {
    QStringList autostartEntries = config["autostart"].toVariant().toStringList();
    QString pathToCurrentDesktopFile = "";
    QString exec = "";

    foreach (QString entry, autostartEntries) {
        pathToCurrentDesktopFile = "/usr/share/applications/" + entry;

        QSettings desktopFileReader(pathToCurrentDesktopFile, QSettings::IniFormat);
        desktopFileReader.sync();
        desktopFileReader.beginGroup("Desktop Entry");
            exec = desktopFileReader.value("Exec").toString();
        desktopFileReader.endGroup();

        /* https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html
         * 'The Exec key' */
        if (exec[exec.length()-2] == "%") {
            exec.chop(2);
        }
        QProcess* process = new QProcess(w);
        process->start(exec);
    }
}

void testpoint(panel* w) {
    // here you can put your code to test

}

panel::panel(QWidget *parent): QWidget(parent) {
    basicInit(this);
    readConfig();
    setPanelGeometry(this);
    setPanelUI(this);
    animation(this);
    autostart(this);

    testpoint(this);
}

panel::~panel() {

}
