#include "panel.h"

#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"
#include "applets/kblayout/kblayout.h"
#include "applets/usermenu/usermenu.h"
#include "applets/volume/volume.h"
#include "applets/windowlist/windowlist.h"
#include "applets/localipv4/localipv4.h"
#include "applets/mpris/mpris.h"
#include "applets/battery/battery.h"
#include "applets/snitray/snitray.h"


AppMenu* mAppMenu;

QList<WId>* mWinIDs = new QList<WId>;
QHash<WId,QPushButton*> mWinWidgets;

DateTimeApplet* mDateTime;

KbLayoutApplet mKbLayout;

UserMenu* mUserMenu;

LocalIPv4Applet mIPv4Applet;

MPRISApplet* mMprisApplet;

BatteryApplet* mBatteryApplet;
Battery mLastBatteryState;
Battery mBatteryState;

qint8 visibleDesktop;
qint8 countWorkspaces;

SNITray* sniTray;
QDBusMessage msg;
QDBusConnection panelSessionBus = QDBusConnection::sessionBus();


QJsonValue Panel::getConfigValue(QString key) {
    return mConfig->value(key);
}

QJsonValue Panel::getConfigValue(QString section, QString key) {
    return mConfig->value(section).toObject()[key];
}

void Panel::setPanelFlags() {
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}

PanelInterference Panel::checkPanelsInterference(PanelLocation loc1, PanelLocation loc2) {
    if (loc1 != loc2) {
        if (loc1 == Top) {
            if (loc2 == Left)
                return TopAndLeft;
            else if (loc2 == Right)
                return TopAndRight;
            else
                return None;
        }

        else if (loc1 == Bottom) {
            if (loc2 == Left)
                return BottomAndLeft;
            else if (loc2 == Right)
                return BottomAndRight;
            else
                return None;
        }

        else if (loc1 == Left) {
            if (loc2 == Top)
                return LeftAndTop;
            else if (loc2 == Bottom)
                return LeftAndBottom;
            else
                return None;
        }

        else { // Right
            if (loc2 == Top)
                return RightAndTop;
            else if (loc2 == Bottom)
                return RightAndBottom;
            else
                return None;
        }
    }
    else {
        return Match;
    }
}

void Panel::setPanelGeometry() {
    // Get primary screen dimensions
    mPrimaryScreen = QGuiApplication::primaryScreen();
    mScreenWidth = mPrimaryScreen->size().width();
    mScreenHeight = mPrimaryScreen->size().height();

    // Panel should update geometry when primary screen resolution is changed
    this->connect(mPrimaryScreen, &QScreen::virtualGeometryChanged, this, [this]() {
        qDebug() << "Screen resolution changed.";
        setPanelGeometry();
    });

    this->connect(mPrimaryScreen, &QScreen::orientationChanged, this, [this]() {
        qDebug() << "Screen resolution changed.";
        setPanelGeometry();
    });

    /* Getting panel thickness
     * Height - horizontal panel
     * Width - vertical panel */
    mPanelThickness = getConfigValue(mPanelName, "thickness").toInt();

    // Location, Layout & Strut
    int ax = 0, ay = 0;
    QString loc = getConfigValue(mPanelName, "location").toString();
    mAxisShift = getConfigValue(mPanelName, "shift").toInt();
    int topStrut = 0, bottomStrut = 0, leftStrut = 0, rightStrut = 0;

    if (loc == "top") {
        mPanelLocation = Top;
        mPanelLayout = Horizontal;
        if (mAxisShift != 0) {
            ax = mAxisShift;
        }

        // This will not work if we have other screen on top of primary screen
        topStrut = mPanelThickness;
    }
    else if (loc == "bottom") {
        mPanelLocation = Bottom;
        mPanelLayout = Horizontal;
        ay = mScreenHeight - mPanelThickness;
        if (mAxisShift != 0) {
            ax = mAxisShift;
        }

        // This will not work if we have other screen on bottom of primary screen
        bottomStrut = mPanelThickness;
    }
    else if (loc == "left") {
        mPanelLocation = Left;
        mPanelLayout = Vertical;
        if (mAxisShift != 0) {
            ay = mAxisShift;
        }

        // This will not work if we have other screen on left of primary screen
        leftStrut = mPanelThickness;
    }
    else {
        mPanelLocation = Right;
        mPanelLayout = Vertical;
        ax = mScreenWidth - mPanelThickness;
        if (mAxisShift != 0) {
            ay = mAxisShift;
        }

        // This will not work if we have other screen on right of primary screen
        rightStrut = mPanelThickness;
    }

    // Size
    if (mPanelLayout == Horizontal) {
        mPanelHeight = mPanelThickness;
        if (getConfigValue(mPanelName, "expand").toBool()) {
            mPanelWidth = mScreenWidth;
        }
        else {
            /* User requested not to stretch panel.
             * We set default panel width to 0, hence it will
             * adjust its width automatically after adding applets. */
            mPanelWidth = 0;
        }
    }
    else {
        mPanelWidth = mPanelThickness;
        if (getConfigValue(mPanelName, "expand").toBool()) {
            mPanelHeight = mScreenHeight;
        }
        else {
            /* User requested not to stretch panel.
             * We set default panel height to 0, hence it will
             * adjust its height automatically after adding applets. */
            mPanelHeight = 0;
        }
    }

    // Avoiding interference of two panels (i.e., left and top)
    for (int i = 0; i < mPrevPanels->length(); ++i) {
        Panel* prevPanel = mPrevPanels->at(i);
        PanelInterference interference = checkPanelsInterference(prevPanel->mPanelLocation,
                                                                 this->mPanelLocation);
        switch (interference) {
        case TopAndLeft: case TopAndRight:
            ay += prevPanel->mPanelThickness;
            mPanelHeight -= prevPanel->mPanelThickness;
            break;
        case BottomAndLeft: case BottomAndRight:
            mPanelHeight -= prevPanel->mPanelThickness;
            break;
        case LeftAndTop: case LeftAndBottom:
            ax += prevPanel->mPanelThickness;
            mPanelWidth -= prevPanel->mPanelThickness;
            break;
        case RightAndTop: case RightAndBottom:
            mPanelWidth -= prevPanel->mPanelThickness;
            break;
        case Match:
            qDebug() << "Panels" << prevPanel->mPanelName << "and" << this->mPanelName << "coincided.";
            break;
        case None:
            break;
        }
    }

    this->setGeometry(ax, ay, mPanelWidth, mPanelHeight);
    this->move(ax, ay);
    this->setFixedSize(mPanelWidth, mPanelHeight);

    KWindowSystem::setStrut(mPanelWId,
                            leftStrut,
                            rightStrut,
                            topStrut,
                            bottomStrut);

    qDebug() << this->geometry().x() << this->geometry().y();
    qDebug() << ax << ay;
    qDebug() << mPanelWidth << mPanelHeight;
}

// Only Time
void Panel::updateDateTime() {
    static_cast<QPushButton*>(mAppletWidgets["dateTimePushButton"])->setText(
                mDateTime->__getDisplayedData__(mTimeFormat));
}

// Date & Time
void Panel::updateDateTime(bool) {
    static_cast<QPushButton*>(mAppletWidgets["dateTimePushButton"])->setText(
                 mDateTime->__getDisplayedData__(mTimeFormat, mDateFormat, mPanelLayout));
}

// ISO code
void Panel::updateKbLayout() {
    static_cast<QPushButton*>(mAppletWidgets["layoutIndicator"])->setText(
                mKbLayout.getCurrentKbLayout());
}

// Flag
void Panel::updateKbLayout(bool) {
    static_cast<QPushButton*>(mAppletWidgets["layoutIndicator"])->setIcon(
                mKbLayout.getCurrentFlag());
}

void Panel::shortenTitle(QString* src, QPushButton* button) {
    int availableWidth = button->geometry().width() - mPanelThickness - 3;
    bool cut = false;
    while (mFontMetrics->horizontalAdvance(*src) > availableWidth) {
        src->chop(1);
        cut = true;
    }
    if (cut)
        src->append("...");
}

void Panel::updateWinList() {
    WindowList::getWinList(mWinIDs);
    for (auto it = mWinIDs->cbegin(), end = mWinIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mPanelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!mWinWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, mPanelThickness, true);
                QString winName = nameInfo.name();
                unsigned short sz = winName.length();
                winName.truncate(12);
                if (winName.length() < sz) {
                    winName += "...";
                }
                QPushButton* windowButton = new QPushButton(winName, this);
                windowButton->setIcon(icon);
                windowButton->setFont(mPanelFont);
                mWinWidgets[*it] = windowButton;
                mWindowListLayout->addWidget(windowButton);

                this->connect(windowButton, &QPushButton::clicked, this, [windowButton]() {
                    KWindowInfo windowInfo(mWinWidgets.key(windowButton), NET::WMState | NET::XAWMState);
                    if (windowInfo.mappingState() != NET::Visible || windowInfo.isMinimized()) {
                        KWindowSystem::unminimizeWindow(mWinWidgets.key(windowButton));
                    }
                    else {
                        KWindowSystem::minimizeWindow(mWinWidgets.key(windowButton));
                    }
                });
            }
            else {
                if (desktopInfo.isOnCurrentDesktop()) {
                    KWindowInfo nameInfo(*it, NET::WMName);
                    QString newName = nameInfo.name();;
                    unsigned short sz = newName.length();
                    newName.truncate(12);
                    if (newName.length() < sz) {
                        newName += "...";
                    }
                    if (mWinWidgets[*it]->text() != newName) {
                        mWinWidgets[*it]->setText(newName);
                    }
                }
                else {
                    delete mWinWidgets[*it];
                    mWinWidgets.remove(*it);
                }
            }
        }
    }
}

void Panel::updateWinList(bool) {
    WindowList::getWinList(mWinIDs);
    for (auto it = mWinIDs->cbegin(), end = mWinIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mPanelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!mWinWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                QPixmap icon = KWindowSystem::icon(*it, -1, mPanelThickness, true);
                QPushButton* windowButton = new QPushButton(this);
                windowButton->setIcon(icon);
                mWinWidgets[*it] = windowButton;
                mWindowListLayout->addWidget(windowButton);

                this->connect(windowButton, &QPushButton::clicked, this, [windowButton]() {
                    KWindowInfo windowInfo(mWinWidgets.key(windowButton), NET::WMState | NET::XAWMState);
                    if (windowInfo.mappingState() != NET::Visible || windowInfo.isMinimized()) {
                        KWindowSystem::unminimizeWindow(mWinWidgets.key(windowButton));
                    }
                    else {
                        KWindowSystem::minimizeWindow(mWinWidgets.key(windowButton));
                    }
                });
            }
            else {
                if (!desktopInfo.isOnCurrentDesktop()) {
                    delete mWinWidgets[*it];
                    mWinWidgets.remove(*it);
                }
            }
        }
    }
}

void Panel::updateWinTitles() {
    for (auto it = mWinIDs->cbegin(), end = mWinIDs->cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mPanelPID) {
            if (mWinWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                unsigned short sz = title.length();
                title.truncate(12);
                if (title.length() < sz) {
                    title += "...";
                }
                if (mWinWidgets[*it]->text() != title) {
                    mWinWidgets[*it]->setText(title);
                }
            }
        }
    }
}

void Panel::accentActiveWindow() {
    WId activeWinID = KWindowSystem::activeWindow();
    foreach (QPushButton* button, mWinWidgets) {
        button->setStyleSheet("");
    }
    if (activeWinID != 0 && mWinWidgets.contains(activeWinID)) {
        mWinWidgets[activeWinID]->setStyleSheet("background-color: " + mAccentColor + "; color: #ffffff;");
    }

}

void Panel::updateWorkspaces() {
    visibleDesktop = KWindowSystem::currentDesktop();
    if (mPanelLayout == Horizontal) {
        for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
            if ((workspace+1) == visibleDesktop) {
                mAppletWidgets["workspace" + QString::number(workspace+1)]->setStyleSheet(
                            "background-color: " + mAccentColor + "; color: #ffffff;");
            }
            else {
                mAppletWidgets["workspace" + QString::number(workspace+1)]->setStyleSheet(
                            "background-color: #9a9996; color: #000000;");
            }
        }
    }
    else {
        static_cast<QPushButton*>(mAppletWidgets["workspace"])->setText(QString::number(visibleDesktop));
    }
}

void Panel::updateLocalIPv4() {
    QString newIP = mIPv4Applet.getLocalIP(mIfname);

    if (newIP != mLastIP) {
        qDebug() << "NEW IP";
        qDebug() << newIP;
        mLastIP = newIP;
        static_cast<QGraphicsTextItem*>(mOtherItems["ipText"])->setPlainText(newIP);
        if (mPanelLayout == Horizontal) {
            short maxWidth = mFontMetrics->horizontalAdvance(newIP);
            static_cast<QGraphicsView*>(mOtherItems["ipView"])->setMaximumWidth(maxWidth);
            static_cast<QGraphicsView*>(mOtherItems["ipView"])->setMinimumWidth(maxWidth);
        }
        else {
            short maxHeight = mFontMetrics->horizontalAdvance(newIP);
            if (mIPAngle == 270) {
                maxHeight += mFontMetrics->horizontalAdvance("A");
            }
            static_cast<QGraphicsView*>(mOtherItems["ipView"])->setMaximumHeight(maxHeight);
            static_cast<QGraphicsView*>(mOtherItems["ipView"])->setMinimumHeight(maxHeight);
        }
    }
}

void Panel::updateBatteryState() {
    mBatteryState = mBatteryApplet->getBatteryState(mBatteryName);

    if (mBatteryState.percentage != mLastBatteryState.percentage) {
        static_cast<QLabel*>(mAppletWidgets["batteryLabel"])->setText(
                    QString::number(mBatteryState.percentage) + "%");

    }
    if (QString::compare(mBatteryState.iconName, mLastBatteryState.iconName)) {
        static_cast<QLabel*>(mAppletWidgets["batteryIcon"])->setPixmap(
                    QIcon("/usr/share/plainDE/icons/" + mBatteryState.iconName).pixmap(
                        mBatteryIconSize, mBatteryIconSize));
    }
}

void Panel::updateBatteryStateDark() {
    mBatteryState = mBatteryApplet->getBatteryState(mBatteryName);

    if (mBatteryState.percentage != mLastBatteryState.percentage) {
        static_cast<QLabel*>(mAppletWidgets["batteryLabel"])->setText(
                    QString::number(mBatteryState.percentage) + "%");

    }
    if (QString::compare(mBatteryState.iconName, mLastBatteryState.iconName)) {
        static_cast<QLabel*>(mAppletWidgets["batteryIcon"])->setPixmap(
                    QIcon("/usr/share/plainDE/icons/" + mBatteryState.iconName + "-dark").pixmap(
                        mBatteryIconSize, mBatteryIconSize));
    }
}

void Panel::setRepeatingActions() {
    // here we bring to life QTimers for updating applets data

    // Date & Time applet

    /* Panel might not start exactly at hh:mm:ss:0000, therefore
     * first update should be earlier. Then we wait 1000 ms between
     * updates. This approach lets us get time more precisely and
     * save resources. */

    if (mActiveAppletsList.contains("datetime")) {
        QTimer* updateDateTimeTimer = new QTimer(this);
        updateDateTimeTimer->setInterval(1000);
        updateDateTimeTimer->setTimerType(Qt::PreciseTimer);
        if (getConfigValue("showDate").toBool()) {
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
            if (getConfigValue("showDate").toBool()) {
                this->updateDateTime(true);
            }
            else {
                this->updateDateTime();
            }
        });
        QTimer::singleShot(delay, Qt::PreciseTimer, updateDateTimeTimer, SLOT(start()));

        mActiveTimers.append(updateDateTimeTimer);
    }


    // Keyboard layout applet
    if (mActiveAppletsList.contains("kblayout")) {
        if (!QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive)) {
            QTimer* updateKbLayoutTimer = new QTimer(this);
            updateKbLayoutTimer->setInterval(350);

            if (getConfigValue("useCountryFlag").toBool()) {
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
            mActiveTimers.append(updateKbLayoutTimer);
        }
        else {
            qDebug() << "Keyboard Layout applet currently works only on X11. Skipping...";
        }
    }


    // Window list applet
    if (mActiveAppletsList.contains("windowlist")) {
        if (!QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive)) {
            if (mPanelLayout == Horizontal) {
                this->connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, [this]() {
                    updateWinList();
                });
            }
            else {
                this->connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, [this]() {
                    updateWinList(false);
                });
            }
            this->connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &Panel::accentActiveWindow);

            if (mPanelLayout == Horizontal) {
                this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
                    this->updateWinList();
                });
            }
            else {
                this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
                    this->updateWinList(false);
                });
            }
            QTimer* updateTitleTimer = new QTimer(this);
            updateTitleTimer->setInterval(1500);
            if (mPanelLayout == Horizontal) {
                this->connect(updateTitleTimer, &QTimer::timeout, this, [this]() {
                    this->updateWinTitles();
                });
            }
            updateTitleTimer->start();
            mActiveTimers.append(updateTitleTimer);
        }
        else {
            qDebug() << "Window List applet currently works only on X11. Skipping...";
        }
    }


    // Workspaces applet
    if (mActiveAppletsList.contains("workspaces")) {
        this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &Panel::updateWorkspaces);
    }

    // Local IP applet
    if (mActiveAppletsList.contains("localipv4")) {
        QTimer* updateLocalIPTimer = new QTimer(this);
        updateLocalIPTimer->setInterval(15000);

        this->connect(updateLocalIPTimer, &QTimer::timeout, this, &Panel::updateLocalIPv4);
        updateLocalIPTimer->start();
        mActiveTimers.append(updateLocalIPTimer);
    }

    // Battery applet
    if (mActiveAppletsList.contains("battery")) {
        QTimer* updateBatteryTimer = new QTimer(this);
        updateBatteryTimer->setInterval(5000);

        if (mStylesheet.contains("dark")) {
            this->connect(updateBatteryTimer, &QTimer::timeout, this, &Panel::updateBatteryState);
        }
        else {
            this->connect(updateBatteryTimer, &QTimer::timeout, this, &Panel::updateBatteryStateDark);
        }

        updateBatteryTimer->start();
        mActiveTimers.append(updateBatteryTimer);
    }

    // SNI tray applet
    if (mActiveAppletsList.contains("sni")) {
        StatusNotifierWatcher* snw = sniTray->mStatusNotifierWatcher;
        this->connect(snw, &StatusNotifierWatcher::StatusNotifierItemRegistered, this, [this, snw]() {
            QString service = snw->registeredStatusNotifierItems().last();
            qDebug() << "Adding icon to SNI layout..." << service;

            QPushButton* sniPushButton = new QPushButton(this);
            sniPushButton->setFlat(true);
            sniPushButton->setIcon(QIcon::fromTheme("dialog-question"));
            mSniWidgets[service] = sniPushButton;
            mSNILayout->addWidget(sniPushButton);

            QtConcurrent::run(sniTray, &SNITray::setSNIIcon, service, sniPushButton);

            this->connect(sniPushButton, &QPushButton::clicked, this, [this, service, sniPushButton]() {
                qDebug() << "Activate";
                msg = QDBusMessage::createMethodCall(service,
                                                     "/StatusNotifierItem",
                                                     "org.kde.StatusNotifierItem",
                                                     "Activate");
                QList<QVariant> args;
                args.append(0);
                args.append(0);

                msg.setArguments(args);
                panelSessionBus.call(msg);
            });
        });

        this->connect(snw, &StatusNotifierWatcher::StatusNotifierItemUnregistered, this, [this, snw]() {
            QString service = snw->deletedItems.last();
            delete mSniWidgets[service];
        });
    }
}

void Panel::setPanelUI() {
#if mPanelLayout == 0
#include <QHBoxLayout>
#else
#include <QVBoxLayout>
#endif

    // Create layout
    if (mPanelLayout == Horizontal) {
        mBoxLayout = new QHBoxLayout(this);
    }
    else {
        mBoxLayout = new QVBoxLayout(this);
    }
    mBoxLayout->setContentsMargins(1, 1, 1, 1);
    mBoxLayout->setSpacing(getConfigValue(mPanelName, "spacing").toInt());

    // Set font
    mPanelFont.setFamily(getConfigValue("fontFamily").toString());
    mPanelFont.setPointSize(getConfigValue("fontSize").toInt());
    this->setFont(mPanelFont);
    mFontMetrics = new QFontMetrics(mPanelFont);

    // Set theme
    mStylesheet = getConfigValue("theme").toString();
    QFile stylesheetReader("/usr/share/plainDE/styles/" + mStylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Set accent
    mAccentColor = getConfigValue("accent").toString();

    // Set opacity
    mOpacity = getConfigValue(mPanelName, "opacity").toDouble();
    this->setWindowOpacity(mOpacity);

    // Set default icon theme
    QIcon::setThemeName(getConfigValue("iconTheme").toString());
}

void Panel::addApplets() {
    // Applets: show applets
    /* We could use QVariantList::contains, but this approach will not save
     * order of placing applets. Using loop. */

    mActiveAppletsList = getConfigValue(mPanelName, "applets").toArray().toVariantList();

    foreach (QVariant applet, mActiveAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            this->layout()->addWidget(appMenuPushButton);
            mAppletWidgets["appMenuPushButton"] = appMenuPushButton;

            QString menuIcon = getConfigValue("menuIcon").toString();
            if (QIcon::hasThemeIcon(menuIcon)) {
                appMenuPushButton->setIcon(QIcon::fromTheme(menuIcon));
            }
            else {
                appMenuPushButton->setIcon(QIcon(menuIcon));
            }

            if (mPanelLayout == Horizontal) {
                QString menuText = getConfigValue("menuText").toString();
                if (!menuText.isEmpty()) {
                    appMenuPushButton->setText(" " + menuText);
                }
            }
        }

        else if (applet == "spacer") {
            if (mPanelLayout == Horizontal) {
                this->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            }
            else {
                this->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
            }
        }

        else if (applet == "splitter") {
            QLabel* splitter = new QLabel((mPanelLayout == Horizontal) ? "|" : "—");
            splitter->setAlignment(Qt::AlignCenter);
            this->layout()->addWidget(splitter);
            mSplitters.append(splitter);
        }

        else if (applet == "datetime") {
            mTimeFormat = getConfigValue("timeFormat").toString();
            mShowDate = getConfigValue("showDate").toBool();
            if (mShowDate) {
                mDateFormat = getConfigValue("dateFormat").toString();
            }

            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            this->layout()->addWidget(dateTimePushButton);

            mAppletWidgets["dateTimePushButton"] = dateTimePushButton;

            if (mShowDate) {
                dateTimePushButton->setText(
                            mDateTime->__getDisplayedData__(mTimeFormat,
                                                           mDateFormat,
                                                           mPanelLayout));
            }
            else {
                dateTimePushButton->setText(
                            mDateTime->__getDisplayedData__(mTimeFormat));
            }
        }

        else if (applet == "kblayout") {
            QPushButton* layoutIndicator = new QPushButton;
            mAppletWidgets["layoutIndicator"] = layoutIndicator;

            short buttonWidth = mFontMetrics->horizontalAdvance("AA");
            if (mPanelLayout == Horizontal) {
                 layoutIndicator->setMaximumWidth(buttonWidth);
            }

            layoutIndicator->setFlat(true);
            layoutIndicator->setIconSize(QSize(16, 16));
            if (mPanelLayout == Horizontal) {
                layoutIndicator->setMaximumWidth(buttonWidth);
            }
            else {
                layoutIndicator->setMaximumHeight(mFontMetrics->height());
            }

            this->layout()->addWidget(layoutIndicator);
        }

        else if (applet == "usermenu") {
            QPushButton* userMenuPushButton = new QPushButton;
            userMenuPushButton->setFlat(true);
            mAppletWidgets["userMenuPushButton"] = userMenuPushButton;
            userMenuPushButton->setObjectName("userMenuPushButton");

            if (mPanelLayout == Horizontal) {
                short userMenuWidth = mFontMetrics->horizontalAdvance(getenv("USER")) + 20;
                userMenuPushButton->setText(getenv("USER"));
                userMenuPushButton->setMaximumWidth(userMenuWidth);
            }
            userMenuPushButton->setIcon(QIcon::fromTheme("computer"));

            this->layout()->addWidget(userMenuPushButton);
        }

        else if (applet == "volume") {
            QDial* volumeDial = new QDial;
            volumeDial->setMinimum(0);
            volumeDial->setMaximum(100);
            volumeDial->setValue(50);
            if (mPanelLayout == Horizontal) {
                volumeDial->setMaximumWidth(25);
            }
            else {
                volumeDial->setMaximumHeight(25);
            }


            QLabel* volumeLabel = new QLabel("50%");
            if (mPanelLayout == Horizontal) {
                volumeLabel->setMaximumWidth(mFontMetrics->horizontalAdvance("100%"));
            }
            volumeLabel->setAlignment(Qt::AlignCenter);

            this->layout()->addWidget(volumeDial);
            this->layout()->addWidget(volumeLabel);

            VolumeApplet::setVolume(50);

            mAppletWidgets["volumeDial"] = volumeDial;
            mAppletWidgets["volumeLabel"] = volumeLabel;
        }

        else if (applet == "windowlist") {
            if (mPanelLayout == Horizontal) {
                mWindowListLayout = new QHBoxLayout();
            }
            else {
                mWindowListLayout = new QVBoxLayout();
            }
            mBoxLayout->addLayout(mWindowListLayout);
        }

        else if (applet == "workspaces") {
            if (mPanelLayout == Horizontal) {
                qint8 countWorkspaces = KWindowSystem::numberOfDesktops();
                for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
                    QPushButton* currentWorkspace = new QPushButton(QString::number(workspace+1));
                    currentWorkspace->setMaximumWidth(mFontMetrics->horizontalAdvance("100"));
                    currentWorkspace->setStyleSheet("background-color: #9a9996; color: #000000;");
                    mAppletWidgets["workspace" + QString::number(workspace+1)] = currentWorkspace;

                    if (KWindowSystem::currentDesktop() == workspace+1) {
                        currentWorkspace->setStyleSheet("background-color: " + mAccentColor + "; color: #ffffff;");
                    }
                    this->layout()->addWidget(currentWorkspace);
                }
            }
            else {
                QPushButton* currentWorkspace = new QPushButton(QString::number(KWindowSystem::currentDesktop()));
                currentWorkspace->setMaximumHeight(mPanelThickness - 2);
                currentWorkspace->setStyleSheet("background-color: " + mAccentColor + "; color: #ffffff;");
                mAppletWidgets["workspace"] = currentWorkspace;
                this->layout()->addWidget(currentWorkspace);
            }
        }

        else if (applet == "localipv4") {
            QGraphicsScene* ipScene = new QGraphicsScene(this);
            QGraphicsTextItem* text = ipScene->addText("0.0.0.0");
            text->setFont(mPanelFont);

            QString color = getConfigValue("ipColor").toString().right(6);

            QList<short> rgb;
            QString block;
            for (int i = 0; i < 6; i += 2) {
                block = QString(color.at(i)) + QString(color.at(i+1));
                rgb.append(block.toInt(NULL, 16));
            }
            text->setDefaultTextColor(QColor::fromRgb(rgb[0], rgb[1], rgb[2]));

            QGraphicsView* view = new QGraphicsView(ipScene, this);
            view->setStyleSheet("background: transparent");

            if (mPanelLayout == Vertical) {
                if (mPanelLocation == Left) {
                    view->rotate(90);
                    mIPAngle = 90;
                }
                else {
                    view->rotate(270);
                    mIPAngle = 270;
                }

                view->setMaximumHeight(mFontMetrics->horizontalAdvance("0.0.0.0"));
                view->setMinimumHeight(mFontMetrics->horizontalAdvance("0.0.0.0"));
            }
            else {
                view->setMaximumWidth(mFontMetrics->horizontalAdvance("0.0.0.0"));
                view->setMinimumWidth(mFontMetrics->horizontalAdvance("0.0.0.0"));
            }

            view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            this->layout()->addWidget(view);
            mOtherItems["ipText"] = text;
            mOtherItems["ipView"] = view;
        }

        else if (applet == "mpris") {
            short buttonWidth = mFontMetrics->horizontalAdvance("⏯") + 2;
            QPushButton* mprisPushButton = new QPushButton("⏯");  // U+23EF - play/pause
            if (mPanelLayout == Horizontal) {
                mprisPushButton->setMaximumWidth(buttonWidth);
            }
            mprisPushButton->setFlat(true);
            mAppletWidgets["mprisPushButton"] = mprisPushButton;
            this->layout()->addWidget(mprisPushButton);
        }

        else if (applet == "battery") {
            mBatteryIconSize = (double)mPanelHeight / 1.45;
            QLabel* batteryIcon = new QLabel;
            mAppletWidgets["batteryIcon"] = batteryIcon;
            QLabel* batteryLabel = new QLabel("100%");
            mAppletWidgets["batteryLabel"] = batteryLabel;
            this->layout()->addWidget(batteryIcon);
            this->layout()->addWidget(batteryLabel);
        }

        else if (applet.toString().startsWith("launcher:")) {
            QStringList launcherData = applet.toString().split(':');

            QString desktopEntryPath = "/usr/share/applications/" + launcherData[1];
            QString exec;
            QString iconPath;
            QString tooltipLabel;

            QSettings desktopFileReader(desktopEntryPath, QSettings::IniFormat);
            desktopFileReader.sync();
            desktopFileReader.beginGroup("Desktop Entry");
                exec = desktopFileReader.value("Exec").toString();
                iconPath = desktopFileReader.value("Icon").toString();
                tooltipLabel = desktopFileReader.value("Name").toString();
            desktopFileReader.endGroup();

            // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html#exec-variables
            if (exec[exec.length()-2] == "%") {
                exec.chop(2);
            }

            short iconSize;
            iconSize = getConfigValue(mPanelName, "launcherIconSize").toInt();
            //iconSize = (double)panelHeight / 1.45;

            QPushButton* launcherPushButton = new QPushButton(this);
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
            launcherPushButton->setToolTip(tooltipLabel);
            launcherPushButton->setToolTipDuration(10000);
            mAppletWidgets[applet.toString()] = launcherPushButton;

            this->connect(launcherPushButton, &QPushButton::clicked, this,
                          [this, exec]() {
                QProcess* process = new QProcess(mExecHolder);
                mProcesses.append(process);
                process->start(exec);
            });

            this->layout()->addWidget(launcherPushButton);
        }

        else if (applet == "sni") {
            if (mPanelLayout == Horizontal) {
                mSNILayout = new QHBoxLayout();
            }
            else {
                mSNILayout = new QVBoxLayout();
            }
            mBoxLayout->addLayout(mSNILayout);
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }

    qDebug() << this;
    this->show();

    // Applets: set actions
    foreach (QVariant applet, mActiveAppletsList) {
        if (applet == "appmenu") {
            int buttonCoord1 = 0, buttonCoord2 = 0;
            if (mPanelLayout == Horizontal) {
                buttonCoord1 = mAppletWidgets["appMenuPushButton"]->x() + mAxisShift;
                buttonCoord2 = mAppletWidgets["appMenuPushButton"]->geometry().topRight().x() + mAxisShift;
            }
            else {
                buttonCoord1 = mAppletWidgets["appMenuPushButton"]->y() + mAxisShift;
                buttonCoord2 = mAppletWidgets["appMenuPushButton"]->geometry().bottomRight().y() + mAxisShift;
            }

            QVariantList favApps = getConfigValue("favApps").toArray().toVariantList();

            mAppMenu = new AppMenu(mExecHolder,
                                   mPanelLocation,
                                   mPanelThickness,
                                   mScreenWidth,
                                   mScreenHeight,
                                   mPanelFont,
                                   buttonCoord1,
                                   buttonCoord2,
                                   getConfigValue("useTriangularTabs").toBool(),
                                   mAccentColor,
                                   mStylesheet,
                                   mOpacity,
                                   &favApps);

            this->connect(static_cast<QPushButton*>(mAppletWidgets["appMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleAppMenu);
        }


        else if (applet == "datetime") {
            // https://code.woboq.org/qt5/qtbase/src/corelib/global/qnamespace.h.html#Qt::DayOfWeek
            Qt::DayOfWeek day = static_cast<Qt::DayOfWeek>(getConfigValue("firstDayOfWeek").toInt());

            int buttonCoord1 = 0, buttonCoord2 = 0;
            if (mPanelLayout == Horizontal) {
                buttonCoord1 = mAppletWidgets["dateTimePushButton"]->x() + mAxisShift;
                buttonCoord2 = mAppletWidgets["dateTimePushButton"]->geometry().topRight().x() + mAxisShift;
            }
            else {
                buttonCoord1 = mAppletWidgets["dateTimePushButton"]->y() + mAxisShift;
                buttonCoord2 = mAppletWidgets["dateTimePushButton"]->geometry().bottomRight().y() + mAxisShift;
            }
            mDateTime = new DateTimeApplet(day,
                                           mPanelLocation,
                                           mPanelFont,
                                           mPanelThickness,
                                           mScreenWidth,
                                           mScreenHeight,
                                           buttonCoord1,
                                           buttonCoord2,
                                           mOpacity);

            this->connect(static_cast<QPushButton*>(mAppletWidgets["dateTimePushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleCalendar);
        }

        else if (applet == "kblayout") {
            mKbLayout.__init__(getConfigValue("kbLayouts").toString().split(','));
            if (getConfigValue("useCountryFlag").toBool()) {
                static_cast<QPushButton*>(mAppletWidgets["layoutIndicator"])->setIcon(
                            QIcon(mKbLayout.getCurrentFlag()));
            }
            else {
                static_cast<QPushButton*>(mAppletWidgets["layoutIndicator"])->setText(mKbLayout.getCurrentKbLayout());
            }
        }

        else if (applet == "usermenu") {
            int buttonCoord1 = 0, buttonCoord2 = 0;
            if (mPanelLayout == Horizontal) {
                buttonCoord1 = mAppletWidgets["userMenuPushButton"]->x() + mAxisShift;
                buttonCoord2 = mAppletWidgets["userMenuPushButton"]->geometry().topRight().x() + mAxisShift;
            }
            else {
                buttonCoord1 = mAppletWidgets["userMenuPushButton"]->y() + mAxisShift;
                buttonCoord2 = mAppletWidgets["userMenuPushButton"]->geometry().bottomRight().y() + mAxisShift;
            }
            mUserMenu = new UserMenu(this,
                                     mExecHolder,
                                     mPanelLocation,
                                     mPanelThickness,
                                     mScreenWidth,
                                     mScreenHeight,
                                     mPanelFont,
                                     buttonCoord1,
                                     buttonCoord2,
                                     mStylesheet,
                                     mOpacity);

            this->connect(static_cast<QPushButton*>(mAppletWidgets["userMenuPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleUserMenu);
        }

        else if (applet == "volume") {
            this->connect(static_cast<QDial*>(mAppletWidgets["volumeDial"]),
                          &QDial::valueChanged,
                          this,
                          &Panel::setVolume);
        }

        else if (applet == "windowlist") {
            this->connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, []() {
                WindowList::getWinList(mWinIDs);
                QList<WId> keys = mWinWidgets.keys();
                foreach (WId id, keys) {
                    if (!mWinIDs->contains(id)) {
                        delete mWinWidgets[id];
                        mWinWidgets.remove(id);
                    }
                }
            });

            if (mPanelLayout == Horizontal) {
                this->updateWinList();
            }
            else {
                this->updateWinList(false);
            }
            this->accentActiveWindow();
        }

        else if (applet == "workspaces") {
            for (qint8 workspace = 0; workspace < countWorkspaces; ++workspace) {
                this->connect(static_cast<QPushButton*>(mAppletWidgets["workspace" + QString::number(workspace+1)]),
                        &QPushButton::clicked, this, [workspace]() {
                    KWindowSystem::setCurrentDesktop(workspace+1);
                });
            }
        }

        else if (applet == "localipv4") {
            this->updateLocalIPv4();
        }

        else if (applet == "mpris") {
            int buttonCoord1 = 0, buttonCoord2 = 0;
            if (mPanelLayout == Horizontal) {
                buttonCoord1 = mAppletWidgets["mprisPushButton"]->x() + mAxisShift;
                buttonCoord2 = mAppletWidgets["mprisPushButton"]->geometry().topRight().x() + mAxisShift;
            }
            else {
                buttonCoord1 = mAppletWidgets["mprisPushButton"]->y() + mAxisShift;
                buttonCoord2 = mAppletWidgets["mprisPushButton"]->geometry().bottomRight().y() + mAxisShift;
            }
            mMprisApplet = new MPRISApplet(mPanelLocation,
                                           mPanelThickness,
                                           mScreenWidth,
                                           mScreenHeight,
                                           buttonCoord1,
                                           buttonCoord2,
                                           mStylesheet,
                                           mOpacity);

            this->connect(static_cast<QPushButton*>(mAppletWidgets["mprisPushButton"]),
                          &QPushButton::clicked,
                          this,
                          &Panel::toggleMPRIS);
        }

        else if (applet == "battery") {
            mBatteryApplet = new BatteryApplet;
            mBatteryName = mBatteryApplet->init();
            if (!mBatteryName.isEmpty()) {
                if (!mStylesheet.contains("dark")) {
                    this->updateBatteryState();
                }
                else {
                    this->updateBatteryStateDark();
                }
            }
            else {
                delete mAppletWidgets["batteryIcon"];
                mAppletWidgets.remove("batteryIcon");
                delete mAppletWidgets["batteryLabel"];
                mAppletWidgets.remove("batteryLabel");
                qDebug() << "Deactivating battery applet (no battery found)...";
                mActiveAppletsList.removeOne("battery");
            }
        }

        else if (applet == "sni") {
            sniTray = new SNITray();
        }

        else {
            // No additional actions for spacer, splitter and launcher applet
            if (applet != "spacer" && applet != "splitter" && !applet.toString().startsWith("launcher")) {
                qDebug() << "Unknown applet:" << applet;
            }
        }
    }

    setRepeatingActions();
}


// Slots
void Panel::toggleAppMenu() {
    if (!mAppMenu->isVisible()) {
        mAppMenu->mTabWidget->setCurrentIndex(0);
        mAppMenu->mSearchBox->clear();
        mAppMenu->buildMenu(mAppMenu->mAppsList, "");
        mAppMenu->show();
    }
    else {
        while (mAppMenu->mAppsList->count() > 0) {
            delete mAppMenu->mAppsList->item(0);
        }
        mAppMenu->mAppsList->clear();
        mAppMenu->hide();
    }
}

void Panel::toggleCalendar() {
    if (!mDateTime->isVisible()) {
        mDateTime->show();
    }
    else {
        mDateTime->hide();
    }
}

void Panel::toggleUserMenu() {
    if (!mUserMenu->isVisible()){
        mUserMenu->show();
    }
    else {
        mUserMenu->hide();
    }
}

void Panel::toggleMPRIS() {
    for (int i = 0; i < mMprisCards->length(); ++i) {
        delete mMprisCards->at(i);
        mMprisCards->removeAt(i);
    }
    if (!mMprisApplet->isVisible()) {
        mMprisApplet->setPlayerCards(mMprisCards,
                                     mPanelFont,
                                     mStylesheet,
                                     mAccentColor);
        mMprisApplet->show();
    }
    else { 
        mMprisApplet->hide();
    }
}

void Panel::setVolume() {
    short newValue = static_cast<QDial*>(mAppletWidgets["volumeDial"])->value();
    VolumeApplet::setVolume(newValue);
    static_cast<QLabel*>(mAppletWidgets["volumeLabel"])->setText(QString::number(newValue) + "%");
}

void Panel::animation(AnimationType type) {
    if (getConfigValue("enableAnimation").toBool()) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(this, "pos");
        panelAnimation->setDuration(250);

        unsigned short ax = 0, ay = 0;
        if (!getConfigValue("expand").toBool()) {
            if (mPanelLayout == Horizontal) {
                ax = this->geometry().x();
            }
            else if (mPanelLayout == Vertical) {
                ay = this->geometry().y();
            }
        }

        switch (mPanelLocation) {
        case Top:
            if (type == Show) {
                panelAnimation->setStartValue(QPoint(ax, -mPanelThickness));
                panelAnimation->setEndValue(QPoint(ax, 0));
            }
            else {
                panelAnimation->setStartValue(QPoint(ax, 0));
                panelAnimation->setEndValue(QPoint(ax, -mPanelThickness));
            }
            break;
        case Bottom:
            if (type == Show) {
                panelAnimation->setStartValue(QPoint(ax, mScreenHeight));
                panelAnimation->setEndValue(QPoint(ax, mScreenHeight - mPanelThickness));
            }
            else {
                panelAnimation->setStartValue(QPoint(ax, mScreenHeight - mPanelThickness));
                panelAnimation->setEndValue(QPoint(ax, mScreenHeight));
            }
            break;
        case Left:
            if (type == Show) {
                panelAnimation->setStartValue(QPoint(-mPanelThickness, ay));
                panelAnimation->setEndValue(QPoint(0, ay));
            }
            else {
                panelAnimation->setStartValue(QPoint(0, ay));
                panelAnimation->setEndValue(QPoint(-mPanelThickness, ay));
            }
            break;
        case Right:
            if (type == Show) {
                panelAnimation->setStartValue(QPoint(mScreenWidth, ay));
                panelAnimation->setEndValue(QPoint(mScreenWidth - mPanelThickness, ay));
            }
            else {
                panelAnimation->setStartValue(QPoint(mScreenWidth - mPanelThickness, ay));
                panelAnimation->setEndValue(QPoint(mScreenWidth, ay));
            }
            break;
        }

        panelAnimation->start();

        if (type == Hide) {
            this->connect(panelAnimation, &QPropertyAnimation::finished, this, [this]() {
                QThread::msleep(250);
                emit animationFinished();
            });
        }

    }
}

void Panel::testpoint(QObject* parent) {
    // parent is Initializer class instance

    /* If you want to get config.json value, use getConfigValue function
     * It can be used like this:
     *  getConfigValue(entry)
     * if you want to get an outside entry (such as iconTheme, menuText, ...)
     * Or like this:
     *  getConfigValue(mPanelName, entry)
     * if you want to get an inner entry of this panel (such as spacing, applets, ...) */

    // here you can put your code to test


}

Panel::Panel(QObject* parent,
             QJsonObject* config ,
             int id,
             QApplication* app,
             QList<Panel*>* prevPanels): QWidget(nullptr) {
    mApplication = app;
    mConfig = config;
    mPanelWId = this->winId();
    KWindowInfo pIDInfo(mPanelWId, NET::WMPid);
    mPanelPID = pIDInfo.pid();
    mPanelName = "panel" + QString::number(id);
    mExecHolder = parent;
    mIfname = getConfigValue("ipIfname").toString();

    // we need this property to avoid interference of two panels
    // (i.e., left and top)
    mPrevPanels = prevPanels;

    mMprisCards = new QList<QWidget*>;

    this->setWindowTitle("plainPanel");
    this->setObjectName("panel");

    setPanelFlags();
    setPanelGeometry();
    setPanelUI();
    addApplets();
    animation(Show);

    this->connect(mUserMenu, &UserMenu::panelShouldQuit, this, [this]() {
        // Some cleaning
        foreach (QProcess* process, mProcesses) {
            delete process;
        }
        mProcesses.clear();

        // Panel hide animation
        animation(Hide);
    });

    testpoint(mExecHolder);
}

Panel::~Panel() {
    // Stop all timers
    foreach (QTimer* currentTimer, mActiveTimers) {
        currentTimer->stop();
        delete currentTimer;
    }

    // Delete App Menu
    if (mActiveAppletsList.contains("appmenu")) {
        mAppMenu->hide();
        while (mAppMenu->mAppsList->count() > 0) {
            delete mAppMenu->mAppsList->item(0);
        }
        delete mAppMenu;
    }

    // Delete calendar
    if (mActiveAppletsList.contains("datetime")) {
        mDateTime->hide();
        delete mDateTime;
    }

    // Delete user menu
    if (mActiveAppletsList.contains("usermenu")) {
        mUserMenu->hide();
        delete mUserMenu;
    }

    // Delete MPRIS
    if (mActiveAppletsList.contains("mpris")) {
        delete mMprisApplet;
        for (int i = 0; i < mMprisCards->length(); ++i) {
            delete mMprisCards->at(i);
            mMprisCards->removeAt(i);
        }
        delete mMprisCards;
    }

    // Delete SNI
    if (mActiveAppletsList.contains("sni")) {
        delete sniTray;
        foreach (QWidget* sniPB, mSniWidgets.values()) {
            delete sniPB;
        }
        delete mSNILayout;
    }

    // Delete all windows from Window List applet (if active)
    if (mActiveAppletsList.contains("windowlist")) {
        foreach (QPushButton* currentButton, mWinWidgets) {
            delete currentButton;
        }
        delete mWindowListLayout;
    }

    // Delete mSplitters
    foreach (QLabel* currentLabel, mSplitters) {
        delete currentLabel;
    }

    // Delete all widgets from panel
    foreach (QWidget* currentWidget, mAppletWidgets) {
        delete currentWidget;
    }

    // Delete layout
    delete mBoxLayout;

    // Delete other stuff
    delete mFontMetrics;

    // Clear variables
    mAppletWidgets.clear();
    mActiveAppletsList.clear();
    mActiveTimers.clear();
    mSplitters.clear();
    mWinIDs->clear();
    mWinWidgets.clear();
    mProcesses.clear();

    this->hide();

    qDebug() << mPanelName << "destructed.";
}
