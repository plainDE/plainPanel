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
#include "applets/battery/battery.h"


QJsonObject* globalConfig;

QMap<QString,QWidget*> appletWidgets;
QList<QLabel*> splitters;
PanelLocation location;
QFont panelFont;
WId panelWinID;
pid_t panelPID;
QScreen* primaryScreen;

unsigned short panelHeight, panelWidth;

QVariantList activeAppletsList;
QHash<QString,QString> panelNameByApplet;
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
QString ipIfname;

MPRISApplet* mprisApplet;
QWidget* mprisWidget;

BatteryApplet* batteryApplet;
QString batteryName;
Battery lastBatteryState;
Battery batteryState;
qint8 batteryIconSize;

qint8 visibleDesktop;
qint8 countWorkspaces;

QString accent;

QString panelName;


QJsonValue Panel::getConfigValueFromSection(QJsonObject* _config, QString section, QString key) {
    return _config->value(section).toObject()[key];
}

void Panel::basicInit(QJsonObject* config, qint8 number) {
    this->setWindowTitle("plainPanel");
    this->setObjectName("panel");

    QDir::setCurrent(getenv("HOME"));

    panelWinID = this->winId();
    KWindowInfo pIDInfo(panelWinID, NET::WMPid);
    panelPID = pIDInfo.pid();

    visibleDesktop = KWindowSystem::currentDesktop();
    countWorkspaces = KWindowSystem::numberOfDesktops();

    primaryScreen = QGuiApplication::primaryScreen();

    panelName = "panel" + QString::number(number);
    globalConfig = config;
}

void Panel::setPanelGeometry() {
    // size, location, monitor settings, window flags

    // Get screens
    //QList<QScreen*> screensList = QGuiApplication::screens();
    // QGuiApplication::primaryScreen()->name();

    // Size & location
    unsigned short ax = 0, ay = 0;
    panelWidth = primaryScreen->size().width();
    panelHeight = getConfigValueFromSection(globalConfig, panelName, "height").toInt();
    qint8 topStrut = panelHeight, bottomStrut = 0;

    if (getConfigValueFromSection(globalConfig, panelName, "location").toString() == "bottom") {
        ay = primaryScreen->size().height() - panelHeight;
        topStrut = 0;
        bottomStrut = panelHeight;
    }

    this->setFixedHeight(panelHeight);
    this->setMaximumWidth(panelWidth);
    if (getConfigValueFromSection(globalConfig, panelName, "expand").toBool()) {
        this->setFixedWidth(panelWidth);
    }
    else {
        ax = getConfigValueFromSection(globalConfig, panelName, "xOffset").toInt();
    }

    qDebug() << "Coordinates: " << ax << " " << ay;
    this->move(ax, ay);

    // _NET_WM_STRUT - Bugfix #4
    KWindowSystem::setStrut(panelWinID, 0, 0, topStrut, bottomStrut);

    // Flags
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}

void Panel::updateGeometry() {
    panelWidth = primaryScreen->size().width();

    // panel is not expanded
    this->setGeometry(this->geometry().x(),
                      this->geometry().y(),
                      minWidth,
                      panelHeight);
}

// Only Time
void Panel::updateDateTime() {
    static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                _dateTime.__getDisplayedData__(timeFormat));
}

// Date & Time
void Panel::updateDateTime(bool) {
    static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                 _dateTime.__getDisplayedData__(timeFormat, dateFormat));
}

// ISO code
void Panel::updateKbLayout() {
    static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(
                _kbLayout.getCurrentKbLayout());
}

// Flag
void Panel::updateKbLayout(bool) {
    static_cast<QPushButton*>(appletWidgets["layoutName"])->setIcon(
                _kbLayout.getCurrentFlag());
}

void Panel::updateWinList() {
    WindowList::getWinList(winIDs);
    for (auto it = winIDs->cbegin(), end = winIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != panelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!winWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, panelHeight, true);
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

void Panel::updateWinTitles() {
    for (auto it = winIDs->cbegin(), end = winIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != panelPID) {
            if (winWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                unsigned short sz = title.length();
                title.truncate(15);
                if (title.length() < sz) {
                    title += "...";
                }
                if (winWidgets[*it]->text() != title) {
                    winWidgets[*it]->setText(title);
                }
            }
        }
    }
}

void Panel::accentActiveWindow() {
    WId activeWinID = KWindowSystem::activeWindow();
    foreach (QPushButton* button, winWidgets) {
        button->setStyleSheet("");
    }
    if (activeWinID != 0 && winWidgets.contains(activeWinID)) {
        winWidgets[activeWinID]->setStyleSheet("background-color: " + accent + "; color: #ffffff;");
    }

}


void Panel::updateWorkspaces() {
    this->updateWinList();
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

void Panel::updateLocalIPv4() {
    static_cast<QLabel*>(appletWidgets["ipLabel"])->setText(
        ipv4Applet.getLocalIP(ipIfname)
    );
}

void Panel::updateBatteryState() {
    batteryState = batteryApplet->getBatteryState(batteryName);

    if (batteryState.percentage != lastBatteryState.percentage) {
        static_cast<QLabel*>(appletWidgets["batteryLabel"])->setText(
                    QString::number(batteryState.percentage) + "%");

    }
    if (QString::compare(batteryState.iconName, lastBatteryState.iconName)) {
        static_cast<QLabel*>(appletWidgets["batteryIcon"])->setPixmap(
                    QIcon("/usr/share/plainDE/icons/" + batteryState.iconName).pixmap(
                        batteryIconSize, batteryIconSize));
    }
}

void Panel::updateBatteryStateDark() {
    batteryState = batteryApplet->getBatteryState(batteryName);

    if (batteryState.percentage != lastBatteryState.percentage) {
        static_cast<QLabel*>(appletWidgets["batteryLabel"])->setText(
                    QString::number(batteryState.percentage) + "%");

    }
    if (QString::compare(batteryState.iconName, lastBatteryState.iconName)) {
        static_cast<QLabel*>(appletWidgets["batteryIcon"])->setPixmap(
                    QIcon("/usr/share/plainDE/icons/" + batteryState.iconName + "-dark").pixmap(
                        batteryIconSize, batteryIconSize));
    }
}

void Panel::setRepeatingActions() {
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
        if (globalConfig->value("showDate").toBool()) {
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
            if (globalConfig->value("showDate").toBool()) {
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

            if (globalConfig->value("useCountryFlag").toBool()) {
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
            this->connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &Panel::updateWinList);
            this->connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &Panel::accentActiveWindow);

            QTimer* updateTitleTimer = new QTimer(this);
            updateTitleTimer->setInterval(2000);
            this->connect(updateTitleTimer, &QTimer::timeout, this, [this](){
                this->updateWinTitles();
            });
            updateTitleTimer->start();
            activeTimers.append(updateTitleTimer);
        }
        else {
            qDebug() << "Window List applet currently works only on X11. Skipping...";
        }
    }


    // Workspaces applet
    if (activeAppletsList.contains("workspaces")) {
        /*QTimer* updateWorkspacesTimer = new QTimer(this);
        updateWorkspacesTimer->setInterval(400);

        this->connect(updateWorkspacesTimer, &QTimer::timeout, this, [this]() {
           this->updateWorkspaces();
        });
        updateWorkspacesTimer->start();
        activeTimers.append(updateWorkspacesTimer);*/

        this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
            qDebug() << "WORKSPACE";
            this->updateWorkspaces();
        });
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

    // Battery applet
    if (activeAppletsList.contains("battery")) {
        QTimer* updateBatteryTimer = new QTimer(this);
        updateBatteryTimer->setInterval(5000);

        if (!globalConfig->value("theme").toString().contains("dark")) {
            this->connect(updateBatteryTimer, &QTimer::timeout, this, &Panel::updateBatteryState);
        }
        else {
            this->connect(updateBatteryTimer, &QTimer::timeout, this, &Panel::updateBatteryStateDark);
        }

        updateBatteryTimer->start();
        activeTimers.append(updateBatteryTimer);
    }


    /* We need to update geometry always if panel is not expanded and
     * WindowList applet used */
    if (!getConfigValueFromSection(globalConfig, panelName, "expandPanel").toBool()) {
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

void Panel::setPanelUI(QObject* execHolder) {
    // Set font
    panelFont.setFamily(globalConfig->value("fontFamily").toString());
    panelFont.setPointSize(globalConfig->value("fontSize").toInt());
    this->setFont(panelFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + globalConfig->value("theme").toString());
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Set accent
    accent = globalConfig->value("accent").toString();

    // Set opacity
    this->setWindowOpacity(getConfigValueFromSection(globalConfig, panelName, "opacity").toDouble());

    // Set network interface
    ipIfname = globalConfig->value("ipIfname").toString();

    // Icons
    QIcon::setThemeName(globalConfig->value("iconTheme").toString());

    // Layout
    QHBoxLayout* uiLayout = new QHBoxLayout;
    uiLayout->setContentsMargins(1, 1, 1, 1);
    this->setLayout(uiLayout);

    // Applets: show applets
    QFontMetrics fm(panelFont);
    location = (getConfigValueFromSection(globalConfig, panelName, "location").toString() == "top") ? top : bottom;

    /* We could use QVariantList::contains, but this approach will not save
     * order of placing applets. Using loop. */

    activeAppletsList = getConfigValueFromSection(globalConfig, panelName, "applets").toArray().toVariantList();

    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            this->layout()->addWidget(appMenuPushButton);
            appletWidgets["appMenuPushButton"] = appMenuPushButton;

            if (QIcon::hasThemeIcon(globalConfig->value("menuIcon").toString())) {
                appMenuPushButton->setIcon(QIcon::fromTheme(globalConfig->value("menuIcon").toString()));
            }
            else {
                appMenuPushButton->setIcon(QIcon(globalConfig->value("menuIcon").toString()));
            }

            appMenuPushButton->setText(" " + globalConfig->value("menuText").toString());
            panelNameByApplet["appmenu"] = panelName;
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
            timeFormat = globalConfig->value("timeFormat").toString();
            if (globalConfig->value("showDate").toBool()) {
                dateFormat = globalConfig->value("dateFormat").toString();
            }

            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            this->layout()->addWidget(dateTimePushButton);

            appletWidgets["dateTimePushButton"] = dateTimePushButton;

            if (globalConfig->value("showDate").toBool()) {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(globalConfig->value("timeFormat").toString(),
                                                           globalConfig->value("dateFormat").toString()));
            }
            else {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(globalConfig->value("timeFormat").toString()));
            }

            panelNameByApplet["datetime"] = panelName;
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
            panelNameByApplet["usermenu"] = panelName;
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
            panelNameByApplet["mpris"] = panelName;
        }

        else if (applet == "battery") {
            batteryIconSize = (double)panelHeight / 1.45;
            QLabel* batteryIcon = new QLabel;
            appletWidgets["batteryIcon"] = batteryIcon;
            QLabel* batteryLabel = new QLabel("100%");
            appletWidgets["batteryLabel"] = batteryLabel;
            this->layout()->addWidget(batteryIcon);
            this->layout()->addWidget(batteryLabel);
            panelNameByApplet["batteryIcon"] = panelName;
            panelNameByApplet["batteryLabel"] = panelName;
        }

        else if (applet.toString().startsWith("launcher:")) {
            QStringList launcherData = applet.toString().split(':');

            QString desktopEntryPath = "/usr/share/applications/" + launcherData[1];
            QString exec;
            QString iconPath;

            QSettings desktopFileReader(desktopEntryPath, QSettings::IniFormat);
            desktopFileReader.sync();
            desktopFileReader.beginGroup("Desktop Entry");
                exec = desktopFileReader.value("Exec").toString();
                iconPath = desktopFileReader.value("Icon").toString();
            desktopFileReader.endGroup();

            /* https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html
             * 'The Exec key' */
            if (exec[exec.length()-2] == "%") {
                exec.chop(2);
            }


            short iconSize;
            iconSize = getConfigValueFromSection(globalConfig, panelName, "launcherIconSize").toInt();
            //iconSize = (double)panelHeight / 1.45;

            QPushButton* launcherPushButton = new QPushButton;
            if (QIcon::hasThemeIcon(iconPath)) {
                launcherPushButton->setIcon(QIcon::fromTheme(iconPath));
            }
            else {
                if (QFile::exists(iconPath)) {
                    launcherPushButton->setIcon(QIcon(iconPath));
                }
                else {
                    launcherPushButton->setIcon(QIcon::fromTheme("dialog-question"));
                }
            }
            launcherPushButton->setIconSize(QSize(iconSize, iconSize));
            launcherPushButton->setFlat(true);

            this->connect(launcherPushButton, &QPushButton::clicked, this,
                          [execHolder, exec]() {
                QProcess* process = new QProcess(execHolder);
                process->start(exec);
            });

            this->layout()->addWidget(launcherPushButton);
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }

    qDebug() << this;
    this->show();

    // Applets: set actions
    foreach (QVariant applet, activeAppletsList) {
        if (applet == "appmenu") {
            _menuUI = menu->__createUI__(execHolder,
                                         location,
                                         panelHeight,
                                         panelFont,
                                         appletWidgets["appMenuPushButton"]->x(),
                                         appletWidgets["appMenuPushButton"]->geometry().topRight().x(),
                                         globalConfig->value("appMenuTriangularTabs").toBool(), accent,
                                         globalConfig->value("theme").toString(),
                                         getConfigValueFromSection(globalConfig, panelName, "opacity").toDouble());
            this->connect(static_cast<QPushButton*>(appletWidgets["appMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleAppMenu);
        }


        else if (applet == "datetime") {
            // https://code.woboq.org/qt5/qtbase/src/corelib/global/qnamespace.h.html#Qt::DayOfWeek
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(globalConfig->value("firstDayOfWeek").toInt());

            _dateTimeUI = _dateTime.__createUI__(location,
                                                 panelHeight,
                                                 panelFont,
                                                 appletWidgets["dateTimePushButton"]->pos().x(),
                                                 appletWidgets["dateTimePushButton"]->geometry().topRight().x(),
                                                 day);

            this->connect(static_cast<QPushButton*>(appletWidgets["dateTimePushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleCalendar);
        }

        else if (applet == "kblayout") {
            _kbLayout.__init__(globalConfig->value("kbLayouts").toString().split(','));
            if (globalConfig->value("useCountryFlag").toBool()) {
                static_cast<QPushButton*>(appletWidgets["layoutName"])->setIcon(
                            QIcon(_kbLayout.getCurrentFlag()));
            }
            else {
                static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
            }
        }

        else if (applet == "usermenu") {
            _userMenuUI = _userMenu.__createUI__(execHolder,
                                                 location,
                                                 panelHeight,
                                                 panelFont,
                                                 appletWidgets["userMenuPushButton"]->pos().x(),
                                                 appletWidgets["userMenuPushButton"]->geometry().topRight().x(),
                                                 globalConfig->value("theme").toString(),
                                                 getConfigValueFromSection(globalConfig, panelName, "opacity").toDouble());

            this->connect(static_cast<QPushButton*>(appletWidgets["userMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleUserMenu);
        }

        else if (applet == "volume") {
            this->connect(static_cast<QDial*>(appletWidgets["volumeDial"]),
                          &QDial::valueChanged,
                          this,
                          &Panel::setVolume);
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
            this->accentActiveWindow();
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
            mprisWidget = mprisApplet->createUI(getConfigValueFromSection(globalConfig, panelName, "location").toString(),
                                                panelHeight,
                                                panelFont,
                                                appletWidgets["mprisPushButton"]->x(),
                                                appletWidgets["mprisPushButton"]->geometry().topRight().x(),
                                                globalConfig->value("theme").toString(),
                                                getConfigValueFromSection(globalConfig, panelName, "opacity").toDouble(),
                                                accent);

            this->connect(static_cast<QPushButton*>(appletWidgets["mprisPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleMPRIS);
        }

        else if (applet == "battery") {
            batteryApplet = new BatteryApplet;
            batteryName = batteryApplet->init();
            if (!batteryName.isEmpty()) {
                if (!globalConfig->value("theme").toString().contains("dark")) {
                    this->updateBatteryState();
                }
                else {
                    this->updateBatteryStateDark();
                }
            }
            else {
                delete appletWidgets["batteryIcon"];
                appletWidgets.remove("batteryIcon");
                delete appletWidgets["batteryLabel"];
                appletWidgets.remove("batteryLabel");
                qDebug() << "Deactivating battery applet (no battery found)...";
                activeAppletsList.removeOne("battery");
            }
        }

        else {
            // No additional actions for spacer, splitter and launcher applet
            if (applet != "spacer" && applet != "splitter" && !applet.toString().startsWith("launcher")) {
                qDebug() << "Unknown applet:" << applet;
            }
        }
    }

    minWidth = this->width();
    setRepeatingActions();
}


// Slots
void Panel::toggleAppMenu() {
    if (!_menuUI.menuWidget->isVisible()) {
        _menuUI.searchBox->clear();
        menu->buildMenu(_menuUI.appListWidget, "");
        menu->buildFavMenu(_menuUI.favListWidget, globalConfig->value("favApps").toArray().toVariantList());
        _menuUI.tabWidget->setCurrentIndex(0);

        short ax;
        short offset = 0;
        if (!getConfigValueFromSection(globalConfig, panelNameByApplet["appmenu"], "expand").toBool()) {
            offset = getConfigValueFromSection(globalConfig, panelNameByApplet["appmenu"], "xOffset").toInt();
        }
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

void Panel::toggleCalendar() {
    if (!_dateTimeUI.calendarWidget->isVisible()) {
        short ax;
        short offset = 0;
        if (!getConfigValueFromSection(globalConfig, panelNameByApplet["datetime"], "expand").toBool()) {
            offset = getConfigValueFromSection(globalConfig, panelNameByApplet["datetime"], "xOffset").toInt();
        }
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

void Panel::toggleUserMenu() {
    if (!_userMenuUI.userMenuWidget->isVisible()) {
        QFontMetrics fm(panelFont);
        short userMenuWidth = fm.horizontalAdvance("About plainDE") + 20;
        short ax;
        short offset = 0;
        if (!getConfigValueFromSection(globalConfig, panelNameByApplet["usermenu"], "expand").toBool()) {
            offset = getConfigValueFromSection(globalConfig, panelNameByApplet["usermenu"], "xOffset").toInt();
        }
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

void Panel::toggleMPRIS() {
    if (!mprisWidget->isVisible()) {
        delete mprisApplet;
        delete mprisWidget;
        mprisApplet = new MPRISApplet;
        mprisWidget = mprisApplet->createUI(getConfigValueFromSection(globalConfig, panelNameByApplet["mpris"], "location").toString(),
                                            panelHeight,
                                            panelFont,
                                            appletWidgets["mprisPushButton"]->x(),
                                            appletWidgets["mprisPushButton"]->geometry().topRight().x(),
                                            globalConfig->value("theme").toString(),
                                            getConfigValueFromSection(globalConfig, panelName, "opacity").toDouble(),
                                            accent);
        mprisWidget->show();
    }
    else {
        mprisWidget->hide();
    }
}

void Panel::filterAppsList() {
    menu->buildMenu(_menuUI.appListWidget, _menuUI.searchBox->text());
}

void Panel::setVolume() {
    short newValue = static_cast<QDial*>(appletWidgets["volumeDial"])->value();
    VolumeApplet::setVolume(newValue);
    static_cast<QLabel*>(appletWidgets["volumeLabel"])->setText(QString::number(newValue) + "%");
}


void Panel::animation() {
    if (globalConfig->value("enableAnimation").toBool()) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(this, "pos");
        panelAnimation->setDuration(250);

        QScreen* primaryScreen = QGuiApplication::primaryScreen();

        unsigned short ax = 0;
        if (!getConfigValueFromSection(globalConfig, panelName, "expand").toBool()) {
            ax = getConfigValueFromSection(globalConfig, panelName, "xOffset").toInt();
        }

        if (getConfigValueFromSection(globalConfig, panelName, "location").toString() == "top") {
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

void Panel::testpoint(QObject* parent, QJsonObject* config) {
    // parent is Initializer class instance
    // config is ~/.config/plainDE/config.json

    // here you can put your code to test
}

Panel::Panel(QWidget* parent,
             QObject* execHolder,
             QJsonObject* _config,
             qint8 number): QWidget(parent) {
    basicInit(_config, number);
    setPanelGeometry();
    setPanelUI(execHolder);
    animation();

    testpoint(execHolder, _config);
}

Panel::~Panel() {
    qDebug() << "DEBUG POINT 1";
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
    qDebug() << "DEBUG POINT 2";

    // Delete all widgets from panel
    foreach (QWidget* currentWidget, appletWidgets) {
        delete currentWidget;
    }
    qDebug() << "DEBUG POINT 3";


    // Delete layout
    delete this->layout();

    // Clear variables
    appletWidgets.clear();
    activeAppletsList.clear();
    activeTimers.clear();
    splitters.clear();
    winIDs->clear();
    winWidgets.clear();
    panelNameByApplet.clear();

    this->hide();
}
