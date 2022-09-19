#include "panel.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QPropertyAnimation>

#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QDial>
#include <QHBoxLayout>

#include <KWindowSystem>

#include "applet.h"
#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"
#include "applets/kblayout/kblayout.h"
#include "applets/usermenu/usermenu.h"
#include "applets/volume/volume.h"
#include "applets/windowlist/windowlist.h"
#include "applets/localipv4/localipv4.h"
#include "applets/mpris/mpris.h"

#include "dbusintegration.h"


QJsonObject config;
QMap<QString,QWidget*> appletWidgets;
QList<QLabel*> splitters;
PanelLocation location;
QFont panelFont;
WId panelWinID;
pid_t panelPID;
QScreen* primaryScreen;

unsigned short panelHeight, panelWidth;

QVariantList activeAppletsList;
QList<QTimer*> activeTimers;

AppMenu* menu;
menuUI _menuUI;

QHBoxLayout* windowListLayout;
QList<WId>* winIDs = new QList<WId>;
QHash<WId,QPushButton*> winWidgets;
unsigned short minWidth;

DateTimeApplet _dateTime;
dateTimeUI _dateTimeUI;
QString timeFormat, dateFormat;

KbLayoutApplet _kbLayout;

UserMenuApplet _userMenu;
userMenuUI _userMenuUI;

LocalIPv4Applet ipv4Applet;

MPRISApplet* mprisApplet;
QWidget* mprisWidget;

qint8 visibleDesktop;
qint8 countWorkspaces;

QString accent;


void panel::readConfig() {
    // set globally readable variable for reading settings

    QString homeDirectory = getenv("HOME");
    QFile file;
    QString data;

    if (!QFile::exists(homeDirectory + "/.config/plainDE/config.json")) {
        qDebug() << homeDirectory + "/.config/plainDE/config.json" + " does not exist. Generating new...";
        system("python3 /usr/share/plainDE/tools/genconfig.py");
    }
    else {
        system("python3 /usr/share/plainDE/tools/update-config.py");
    }

    file.setFileName(homeDirectory + "/.config/plainDE/config.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data = file.readAll();
    file.close();
    config = QJsonDocument::fromJson(data.toUtf8()).object();
}

void panel::basicInit() {
    if (QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive) != 0) {
        qDebug() << "plainPanel currently works on X11 only. Quitting...";
        exit(0);
    }

    this->setWindowTitle("plainPanel");
    this->setObjectName("panel");

    QDir::setCurrent(getenv("HOME"));

    panelWinID = this->winId();
    KWindowInfo pIDInfo(panelWinID, NET::WMPid);
    panelPID = pIDInfo.pid();

    visibleDesktop = KWindowSystem::currentDesktop();
    countWorkspaces = KWindowSystem::numberOfDesktops();

    primaryScreen = QGuiApplication::primaryScreen();

    DBusIntegration db("org.plainDE.plainPanel", "/Actions", "org.plainDE.actions", this);
}

void panel::setPanelGeometry() {
    // size, location, monitor settings, window flags

    // Get screens
    // QList<QScreen*> screensList = QGuiApplication::screens();
    // QGuiApplication::primaryScreen()->name();

    // Size & location
    unsigned short ax = 0, ay = 0;
    panelWidth = primaryScreen->size().width();
    panelHeight = config["panelHeight"].toInt();
    qint8 topStrut = panelHeight, bottomStrut = 0;

    if (config["panelLocation"].toString() == "bottom") {
        ay = primaryScreen->size().height() - panelHeight;
        topStrut = 0;
        bottomStrut = panelHeight;
    }

    this->setFixedHeight(panelHeight);
    this->setMaximumWidth(panelWidth);
    if (config["expandPanel"].toBool()) {
        this->setFixedWidth(panelWidth);
    }
    else {
        ax = config["xOffset"].toInt();
    }

    qDebug() << "Coordinates: " << ax << " " << ay;
    this->move(ax, ay);

    // _NET_WM_STRUT - Bugfix #4
    KWindowSystem::setStrut(panelWinID, 0, 0, topStrut, bottomStrut);

    // Moving panel on other workspaces - Bugfix #3
    this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, []() {
        KWindowSystem::setOnDesktop(panelWinID, KWindowSystem::currentDesktop());
    });

    // Flags
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}

void panel::updateGeometry() {
    panelWidth = primaryScreen->size().width();

    // panel is not expanded
    this->setGeometry(this->geometry().x(),
                      this->geometry().y(),
                      minWidth,
                      panelHeight);
}

// Only Time
void panel::updateDateTime() {
    static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                _dateTime.__getDisplayedData__(timeFormat));
}

// Date & Time
void panel::updateDateTime(bool) {
    static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                 _dateTime.__getDisplayedData__(timeFormat, dateFormat));
}

// ISO code
void panel::updateKbLayout() {
    static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(
                _kbLayout.getCurrentKbLayout());
}

// Flag
void panel::updateKbLayout(bool) {
    static_cast<QPushButton*>(appletWidgets["layoutName"])->setIcon(
                _kbLayout.getCurrentFlag());
}

void panel::updateWinList() {
    WindowList::getWinList(winIDs);
    for (auto it = winIDs->cbegin(), end = winIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != panelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!winWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, config["panelHeight"].toInt(), true);
                QString winName = nameInfo.name();
                unsigned short sz = winName.length();
                winName.truncate(15);
                if (winName.length() < sz) {
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
            }
            else {
                if (desktopInfo.isOnCurrentDesktop()) {
                    KWindowInfo nameInfo(*it, NET::WMName);
                    QString newName = nameInfo.name();;
                    unsigned short sz = newName.length();
                    newName.truncate(15);
                    if (newName.length() < sz) {
                        newName += "...";
                    }
                    if (winWidgets[*it]->text() != newName) {
                        winWidgets[*it]->setText(newName);
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

void panel::updateLocalIPv4() {
    static_cast<QLabel*>(appletWidgets["ipLabel"])->setText(
        ipv4Applet.getLocalIP(config["ipIfname"].toString())
    );
}

void panel::setRepeatingActions() {
    // here we bring to life QTimers for updating applets data

    // Date & Time applet

    /* Panel may not start exactly at hh:mm:ss:0000, therefore
     * first update should be earlier. Then we wait 1000 ms between
     * updates. This approach lets us get time more precisely and
     * save resources. */

    if (activeAppletsList.contains("datetime")) {        
        QTimer* updateDateTimeTimer = new QTimer(this);
        updateDateTimeTimer->setInterval(1000);
        updateDateTimeTimer->setTimerType(Qt::PreciseTimer);
        if (config["showDate"].toBool()) {
            this->connect(updateDateTimeTimer, &QTimer::timeout, this, [this]() {
                this->updateDateTime(true);
            });
        }
        else {
            this->connect(updateDateTimeTimer, &QTimer::timeout, this, [this]() {
                this->updateDateTime();
            });
        }

        // https://github.com/lxqt/lxqt-panel/blob/master/plugin-worldclock/lxqtworldclock.cpp
        unsigned short delay = 1000 - ((QTime::currentTime().msecsSinceStartOfDay()) % 1000);
        QTimer::singleShot(delay, Qt::PreciseTimer, this, [this]() {
            if (config["showDate"].toBool()) {
                this->updateDateTime(true);
            }
            else {
                this->updateDateTime();
            }
        });
        QTimer::singleShot(delay, Qt::PreciseTimer, updateDateTimeTimer, SLOT(start()));

        activeTimers.append(updateDateTimeTimer);
    }


    // Keyboard layout applet
    if (activeAppletsList.contains("kblayout")) {
        if (!QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive)) {
            QTimer* updateKbLayoutTimer = new QTimer(this);
            updateKbLayoutTimer->setInterval(350);

            if (config["useCountryFlag"].toBool()) {
                this->connect(updateKbLayoutTimer, &QTimer::timeout, this, [this]() {
                    this->updateKbLayout(true);
                });
            }
            else {
                this->connect(updateKbLayoutTimer, &QTimer::timeout, this, [this]() {
                    this->updateKbLayout();
                });
            }
            updateKbLayoutTimer->start();
            activeTimers.append(updateKbLayoutTimer);
        }
        else {
            qDebug() << "Keyboard Layout applet currently works only on X11. Skipping...";
        }
    }


    // Window list applet
    if (activeAppletsList.contains("windowlist")) {
        if (!QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive)) {
            QTimer* updateWinListTimer = new QTimer(this);
            updateWinListTimer->setInterval(400);
            this->connect(updateWinListTimer, &QTimer::timeout, this, [this]() {
                this->updateWinList();
            });
            updateWinListTimer->start();
            activeTimers.append(updateWinListTimer);
        }
        else {
            qDebug() << "Window List applet currently works only on X11. Skipping...";
        }
    }


    // Workspaces applet
    if (activeAppletsList.contains("workspaces")) {
        QTimer* updateWorkspacesTimer = new QTimer(this);
        updateWorkspacesTimer->setInterval(400);

        this->connect(updateWorkspacesTimer, &QTimer::timeout, this, [this]() {
           this->updateWorkspaces();
        });
        updateWorkspacesTimer->start();
        activeTimers.append(updateWorkspacesTimer);
    }

    // Local IP applet
    if (activeAppletsList.contains("localipv4")) {
        QTimer* updateLocalIPTimer = new QTimer(this);
        updateLocalIPTimer->setInterval(15000);

        this->connect(updateLocalIPTimer, &QTimer::timeout, this, [this]() {
            this->updateLocalIPv4();
        });
        updateLocalIPTimer->start();
        activeTimers.append(updateLocalIPTimer);
    }


    /* We need to update geometry always if panel is not expanded and
     * WindowList applet used */
    if (!config["expandPanel"].toBool()) {
        /* Window List applet currently works only on X11, so we don't need to update
         * geometry if isn't activated. */
        if (!QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive)) {
            if (activeAppletsList.contains("windowlist")) {
                QTimer* updateGeometryTimer = new QTimer(this);
                updateGeometryTimer->setInterval(500);

                this->connect(updateGeometryTimer, &QTimer::timeout, this, [this]() {
                    this->updateGeometry();
                });
                updateGeometryTimer->start();
                activeTimers.append(updateGeometryTimer);
            }
        }
    }
}

void panel::setPanelUI() {
    // Set font
    panelFont.setFamily(config["fontFamily"].toString());
    panelFont.setPointSize(config["fontSize"].toInt());
    this->setFont(panelFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + config["theme"].toString());
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Set accent
    accent = config["accent"].toString();

    // Set opacity
    this->setWindowOpacity(config["panelOpacity"].toDouble());

    // Icons
    QIcon::setThemeName(config["iconTheme"].toString());

    // Layout
    QHBoxLayout* uiLayout = new QHBoxLayout;
    uiLayout->setContentsMargins(1, 1, 1, 1);
    this->setLayout(uiLayout);

    // Applets: show applets
    QFontMetrics fm(panelFont);
    location = (config["panelLocation"].toString() == "top") ? top : bottom;

    /* We could use QVariantList::contains, but this approach will not save
     * order of placing applets. Using loop. */

    activeAppletsList = config["applets"].toArray().toVariantList();

    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            this->layout()->addWidget(appMenuPushButton);
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
            this->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
        }

        else if (applet == "splitter") {
            QLabel* splitter = new QLabel("|");
            this->layout()->addWidget(splitter);
            splitters.append(splitter);
        }

        else if (applet == "datetime") {
            timeFormat = config["timeFormat"].toString();
            if (config["showDate"].toBool()) {
                dateFormat = config["dateFormat"].toString();
            }

            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            this->layout()->addWidget(dateTimePushButton);

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
            layoutName->setIconSize(QSize(16, 16));

            this->layout()->addWidget(layoutName);
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

            this->layout()->addWidget(userMenuPushButton);
        }

        else if (applet == "volume") {
            QDial* volumeDial = new QDial;
            volumeDial->setMinimum(0);
            volumeDial->setMaximum(100);
            volumeDial->setValue(50);
            volumeDial->setMaximumWidth(25);

            QLabel* volumeLabel = new QLabel("50%");
            volumeLabel->setMaximumWidth(fm.horizontalAdvance("100%"));

            this->layout()->addWidget(volumeDial);
            this->layout()->addWidget(volumeLabel);

            VolumeApplet::setVolume(50);

            appletWidgets["volumeDial"] = volumeDial;
            appletWidgets["volumeLabel"] = volumeLabel;
        }

        else if (applet == "windowlist") {
            windowListLayout = new QHBoxLayout;
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
                this->layout()->addWidget(currentWorkspace);
            }
        }

        else if (applet == "localipv4") {
            QLabel* ipLabel = new QLabel("0.0.0.0");
            this->layout()->addWidget(ipLabel);
            appletWidgets["ipLabel"] = ipLabel;
        }

        else if (applet == "mpris") {
            short buttonWidth = fm.horizontalAdvance("⏯");
            QPushButton* mprisPushButton = new QPushButton("⏯");  // U+23EF - play/pause
            mprisPushButton->setMaximumWidth(buttonWidth);
            mprisPushButton->setFlat(true);
            appletWidgets["mprisPushButton"] = mprisPushButton;
            this->layout()->addWidget(mprisPushButton);
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }
    this->show();

    // Applets: set actions
    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            _menuUI = menu->__createUI__(location, config["panelHeight"].toInt(),
                                        panelFont,
                                        appletWidgets["appMenuPushButton"]->x(),
                                        appletWidgets["appMenuPushButton"]->geometry().topRight().x(),
                                        config["appMenuTriangularTabs"].toBool(), accent,
                                        config["theme"].toString(),
                                        config["panelOpacity"].toDouble());
            this->connect(static_cast<QPushButton*>(appletWidgets["appMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &panel::toggleAppMenu);
        }


        else if (applet == "datetime") {
            // https://code.woboq.org/qt5/qtbase/src/corelib/global/qnamespace.h.html#Qt::DayOfWeek
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(config["firstDayOfWeek"].toInt());

            _dateTimeUI = _dateTime.__createUI__(location,
                                                 config["panelHeight"].toInt(),
                                                 panelFont,
                                                 appletWidgets["dateTimePushButton"]->pos().x(),
                                                 appletWidgets["dateTimePushButton"]->geometry().topRight().x(),
                                                 day);

            this->connect(static_cast<QPushButton*>(appletWidgets["dateTimePushButton"]),
                          &QPushButton::clicked,
                          this,
                          &panel::toggleCalendar);
        }

        else if (applet == "kblayout") {
            _kbLayout.__init__(config["kbLayouts"].toString().split(','));
            if (config["useCountryFlag"].toBool()) {
                static_cast<QPushButton*>(appletWidgets["layoutName"])->setIcon(
                            QIcon(_kbLayout.getCurrentFlag()));
            }
            else {
                static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
            }
        }

        else if (applet == "usermenu") {
            _userMenuUI = _userMenu.__createUI__(location,
                                          config["panelHeight"].toInt(),
                                          panelFont,
                                          appletWidgets["userMenuPushButton"]->pos().x(),
                                          appletWidgets["userMenuPushButton"]->geometry().topRight().x(),
                                          config["theme"].toString(),
                                          config["panelOpacity"].toDouble());

            this->connect(static_cast<QPushButton*>(appletWidgets["userMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &panel::toggleUserMenu);
        }

        else if (applet == "volume") {
            this->connect(static_cast<QDial*>(appletWidgets["volumeDial"]),
                          &QDial::valueChanged,
                          this,
                          &panel::setVolume);
        }

        else if (applet == "windowlist") {
            this->connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, []() {
                WindowList::getWinList(winIDs);
                QList<WId> keys = winWidgets.keys();
                foreach (WId id, keys) {
                    if (!winIDs->contains(id)) {
                        delete winWidgets[id];
                        winWidgets.remove(id);
                    }
                }
            });

            this->updateWinList();
        }

        else if (applet == "workspaces") {
            for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
                this->connect(static_cast<QPushButton*>(appletWidgets["workspace" + QString::number(workspace+1)]),
                        &QPushButton::clicked, this, [workspace]() {
                    KWindowSystem::setCurrentDesktop(workspace+1);
                });
            }
        }

        else if (applet == "localipv4") {
            this->updateLocalIPv4();
        }

        else if (applet == "mpris") {
            mprisApplet = new MPRISApplet;
            mprisWidget = mprisApplet->createUI(location,
                                                config["panelHeight"].toInt(),
                                                panelFont,
                                                appletWidgets["mprisPushButton"]->x(),
                                                appletWidgets["mprisPushButton"]->geometry().topRight().x(),
                                                config["theme"].toString(),
                                                config["panelOpacity"].toDouble(),
                                                accent);

            this->connect(static_cast<QPushButton*>(appletWidgets["mprisPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &panel::toggleMPRIS);
        }

        else {
            // No actions for spacer and splitter applet
            if (applet != "spacer" && applet != "splitter") {
                qDebug() << "Unknown applet:" << applet;
            }
        }
    }

    minWidth = this->width();
    setRepeatingActions();
    this->freeUnusedMemory(false);
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

        _menuUI.menuWidget->move(ax, _menuUI.menuWidget->geometry().y());
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

        _dateTimeUI.calendarWidget->move(ax, _dateTimeUI.calendarWidget->geometry().y());
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

void panel::toggleMPRIS() {
    if (!mprisWidget->isVisible()) {
        delete mprisApplet;
        delete mprisWidget;
        mprisApplet = new MPRISApplet;
        mprisWidget = mprisApplet->createUI(location,
                                            config["panelHeight"].toInt(),
                                            panelFont,
                                            appletWidgets["mprisPushButton"]->x(),
                                            appletWidgets["mprisPushButton"]->geometry().topRight().x(),
                                            config["theme"].toString(),
                                            config["panelOpacity"].toDouble(),
                                            accent);
        mprisWidget->show();
    }
    else {
        mprisWidget->hide();
    }
}

void panel::filterAppsList() {
    menu->buildMenu(_menuUI.appListWidget, _menuUI.searchBox->text());
}

void panel::setVolume() {
    short newValue = static_cast<QDial*>(appletWidgets["volumeDial"])->value();
    VolumeApplet::setVolume(newValue);
    static_cast<QLabel*>(appletWidgets["volumeLabel"])->setText(QString::number(newValue) + "%");
}


void panel::animation() {
    if (config["enableAnimation"].toBool()) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(this, "pos");
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

void panel::autostart() {
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
        QProcess* process = new QProcess(this);
        process->start(exec);
    }


    // setxkbmap
    if (!config["kbLayouts"].toVariant().toStringList().isEmpty() &&
        !config["kbLayoutToggle"].toString().isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(this);
        setxkbmapProcess->start("setxkbmap -layout " + config["kbLayouts"].toString() +
                                " -option " + config["kbLayoutToggle"].toString());
    }
}

void panel::freeUnusedMemory(bool quit) {
    if (!quit) {
        if (!activeAppletsList.contains("windowlist")) {
            delete windowListLayout;
            delete winIDs;
        }
    }
    else {
        if (activeAppletsList.contains("appmenu")) {
            delete menu;
        }
        if (activeAppletsList.contains("windowlist")) {
            QList<WId> keys = winWidgets.keys();
            foreach (WId currentWindow, keys) {
                delete winWidgets[currentWindow];
            }
        }
    }
}

/* We need to clear objects from panel, read configs again
 * when reconfiguring panel */
void panel::reconfigurePanel() {
    // Stop all timers
    foreach (QTimer* currentTimer, activeTimers) {
        currentTimer->stop();
        delete currentTimer;
    }

    // Delete App Menu
    if (activeAppletsList.contains("appmenu")) {
        _menuUI.menuWidget->hide();
        delete menu;
    }

    // Delete calendar
    if (activeAppletsList.contains("datetime")) {
        _dateTimeUI.calendarWidget->hide();
        delete _dateTimeUI.calendarWidget;
    }

    // Delete user menu
    if (activeAppletsList.contains("datetime")) {
        _userMenuUI.userMenuWidget->hide();
        delete _userMenuUI.userMenuWidget;
    }

    // Delete MPRIS
    if (activeAppletsList.contains("mpris")) {
        delete mprisApplet;
        delete mprisWidget;
    }

    // Delete all windows from Window List applet (if active)
    if (activeAppletsList.contains("windowlist")) {
        foreach (QPushButton* currentButton, winWidgets) {
            delete currentButton;
        }
    }

    // Delete splitters
    foreach (QLabel* currentLabel, splitters) {
        delete currentLabel;
    }

    // Delete all widgets from panel
    foreach (QWidget* currentWidget, appletWidgets) {
        delete currentWidget;
    }

    // Delete layout
    delete this->layout();

    // Clear variables
    appletWidgets.clear();
    activeAppletsList.clear();
    activeTimers.clear();
    splitters.clear();
    winIDs->clear();
    winWidgets.clear();

    this->hide();


    this->basicInit();
    this->readConfig();
    this->setPanelGeometry();
    this->setPanelUI();
    this->animation();

    // setxkbmap
    if (!config["kbLayouts"].toVariant().toStringList().isEmpty() &&
        !config["kbLayoutToggle"].toString().isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(this);
        setxkbmapProcess->start("setxkbmap -layout " + config["kbLayouts"].toString() +
                                " -option " + config["kbLayoutToggle"].toString());
    }
}

void panel::testpoint() {
    // here you can put your code to test
}

panel::panel(QWidget *parent): QWidget(parent) {
    basicInit();
    readConfig();
    setPanelGeometry();
    setPanelUI();
    animation();
    autostart();

    testpoint();
}

panel::~panel() {

}
