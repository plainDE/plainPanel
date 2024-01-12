#include "panel.h"

#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"
#include "applets/kblayout/kblayout.h"
#include "applets/usermenu/usermenu.h"
#include "applets/volume/volume.h"
#include "applets/windowlist/windowlist.h"
#include "applets/workspaces/workspaces.h"
#include "applets/launcher/launcher.h"
#include "applets/localipv4/localipv4.h"
#include "applets/mpris/mpris.h"
#include "applets/battery/battery.h"
#include "applets/snitray/snitray.h"
#include "applets/clioutput/clioutput.h"

AppMenuApplet* mAppMenuApplet;
DateTimeApplet* mDateTimeApplet;
KbLayoutApplet* mKbLayoutApplet;
UserMenuApplet* mUserMenuApplet;
VolumeApplet* mVolumeApplet;
WorkspacesApplet* mWorkspacesApplet;
LocalIPv4Applet* mIPv4Applet;
BatteryApplet* mBatteryApplet;
MPRISApplet* mMPRISApplet;
SNITrayApplet* mSNITrayApplet;
WindowListApplet* mWinListApplet;
QList<CLIOutputApplet*> mCLIOutputAppletsList = {};

QDBusMessage msg;
QDBusConnection panelSessionBus = QDBusConnection::sessionBus();


void Panel::setPanelFlags() {
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
}

void Panel::setPanelGeometry() {
    // Set panel screen
    QString screenName = mCfgMan->mPanels.at(mPanelID - 1).screen;
    QList<QScreen*> screens = QGuiApplication::screens();
    foreach (QScreen* screen, screens) {
        if (screen->name() == screenName) {
            mPanelScreen = screen;
        }
    }
    if (!mPanelScreen) {
        mPanelScreen = QGuiApplication::primaryScreen();
    }
    mScreenGeometry = mPanelScreen->geometry();
    mScreenWidth = mPanelScreen->size().width();
    mScreenHeight = mPanelScreen->size().height();

    // Panel should update geometry when screen resolution is changed
    this->connect(mPanelScreen, &QScreen::geometryChanged, this, [this]() {
        qDebug() << "Screen geometry changed.";
        setPanelGeometry();
    });

    /* Getting panel thickness
     * Height - horizontal panel
     * Width - vertical panel */
    mPanelThickness = mCfgMan->mPanels.at(mPanelID - 1).thickness;

    // Location, Layout & Strut
    int ax = mScreenGeometry.x(), ay = mScreenGeometry.y();
    mPanelLocation = mCfgMan->mPanels.at(mPanelID - 1).location;
    mAxisShift = mCfgMan->mPanels.at(mPanelID - 1).shift;
    int topStrut = 0, bottomStrut = 0, leftStrut = 0, rightStrut = 0;

    if (mPanelLocation == Top) {
        mPanelLayout = Horizontal;
        if (mAxisShift != 0) {
            ax += mAxisShift;
        }
        topStrut = mPanelThickness;
    }
    else if (mPanelLocation == Bottom) {
        mPanelLayout = Horizontal;
        ay = mScreenGeometry.y() + mScreenHeight - mPanelThickness;
        if (mAxisShift != 0) {
            ax += mAxisShift;
        }
        bottomStrut = mPanelThickness;
    }
    else if (mPanelLocation == Left) {
        mPanelLayout = Vertical;
        if (mAxisShift != 0) {
            ay += mAxisShift;
        }
        leftStrut = mPanelThickness;
    }
    else {  // Right
        mPanelLayout = Vertical;
        ax = mScreenGeometry.x() + mScreenWidth - mPanelThickness;
        if (mAxisShift != 0) {
            ay += mAxisShift;
        }
        rightStrut = mPanelThickness;
    }

    // Size
    if (mPanelLayout == Horizontal) {
        mPanelHeight = mPanelThickness;
        if (mCfgMan->mPanels.at(mPanelID - 1).expand) {
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
        if (mCfgMan->mPanels.at(mPanelID - 1).expand) {
            mPanelHeight = mScreenHeight;
        }
        else {
            /* User requested not to stretch panel.
             * We set default panel height to 0, hence it will
             * adjust its height automatically after adding applets. */
            mPanelHeight = 0;
        }
    }

    // Avoiding intersection of two panels (i.e., left and top)
    for (int i = 0; i < mPrevPanels.length(); ++i) {
        Panel* prevPanel = mPrevPanels.at(i);
        typedef PanelLocation pl;
        pl l1 = prevPanel->mPanelLocation, l2 = this->mPanelLocation;

        if ((l1 == pl::Top && l2 == pl::Left) ||
            (l1 == pl::Top && l2 == pl::Right)) {
                ay += prevPanel->mPanelThickness;
                mPanelHeight -= prevPanel->mPanelThickness;
        }
        else if ((l1 == pl::Bottom && l2 == pl::Left) ||
                 (l1 == pl::Bottom && l2 == pl::Right)) {
            mPanelHeight -= prevPanel->mPanelThickness;
        }
        else if ((l1 == pl::Left && l2 == pl::Top) ||
                 (l1 == pl::Left && l2 == pl::Bottom)) {
            ax += prevPanel->mPanelThickness;
            mPanelWidth -= prevPanel->mPanelThickness;
        }
        else if ((l1 == pl::Right && l2 == pl::Top) ||
                 (l1 == pl::Right && l2 == pl::Bottom)) {
            mPanelWidth -= prevPanel->mPanelThickness;
        }
        else if (l1 == l2) {
            qDebug() << "Panels" << prevPanel->mPanelID << "and" << this->mPanelID << "coincided.";
        }
    }

    this->setGeometry(ax, ay, mPanelWidth, mPanelHeight);
    this->move(ax, ay);
    this->setFixedSize(mPanelWidth, mPanelHeight);

    KWindowSystem::setExtendedStrut(mPanelWId,
                                    leftStrut,
                                    mScreenGeometry.y(),
                                    mScreenGeometry.y() + mScreenGeometry.height(),
                                    rightStrut,
                                    mScreenGeometry.y(),
                                    mScreenGeometry.y() + mScreenGeometry.height(),
                                    topStrut,
                                    mScreenGeometry.x(),
                                    mScreenGeometry.x() + mScreenGeometry.width(),
                                    bottomStrut,
                                    mScreenGeometry.x(),
                                    mScreenGeometry.x() + mScreenGeometry.width());

    qDebug() << "Strut:" << leftStrut << rightStrut << topStrut << bottomStrut;
    qDebug() << "left strut restrictions:" << mScreenGeometry.y() << '-' << mScreenGeometry.y() + mScreenGeometry.height();
    qDebug() << "right strut restrictions:" << mScreenGeometry.y() << '-' << mScreenGeometry.y() + mScreenGeometry.height();
    qDebug() << "top strut restrictions:" << mScreenGeometry.x() << '-' << mScreenGeometry.x() + mScreenGeometry.width();
    qDebug() << "bottom strut restrictions:" << mScreenGeometry.x() << '-' << mScreenGeometry.x() + mScreenGeometry.width();

    qDebug() << this->geometry().x() << this->geometry().y();
    qDebug() << ax << ay;
    qDebug() << mPanelWidth << mPanelHeight;

    KWindowSystem::setOnAllDesktops(mPanelWId, true);
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
        KWindowSystem::setOnDesktop(mPanelWId, KWindowSystem::currentDesktop());
    });
    mCountWorkspaces = KWindowSystem::numberOfDesktops();
}

void Panel::setRepeatingActions() {
    // here we bring to life QTimers for updating applets data

    const QJsonArray* applets = &mCfgMan->mPanels.at(mPanelID - 1).applets;
    // Date & Time applet
    if (applets->contains("datetime")) {
        mDateTimeApplet->activate(mCfgMan, this);
        if (mCfgMan->mShowDate) {
            mDateTimeApplet->repeatingAction(mCfgMan, this, true);
        }
        else {
            mDateTimeApplet->repeatingAction(mCfgMan, this);
        }
    }

    // Keyboard layout applet
    if (applets->contains("kblayout")) {
        mKbLayoutApplet->activate(mCfgMan, this);
        if (mCfgMan->mUseCountryFlag) {
            mKbLayoutApplet->repeatingAction(mCfgMan, this, true);
        }
        else {
            mKbLayoutApplet->repeatingAction(mCfgMan, this);
        }
    }

    // Local IPv4 applet
    if (applets->contains("localipv4")) {
        mIPv4Applet->activate(mCfgMan, this);
        mIPv4Applet->repeatingAction(mCfgMan, this);
    }

    // Battery applet
    if (applets->contains("battery") && mBatteryApplet->mDeviceHasBattery) {
        mBatteryApplet->activate(mCfgMan, this);
        mBatteryApplet->repeatingAction(mCfgMan, this);
    }

    // Window List applet
    if (applets->contains("windowlist")) {
        mWinListApplet->activate(mCfgMan, this);
        mWinListApplet->addButtons(mCfgMan, this);
        mWinListApplet->accentActiveWindow(mCfgMan);
    }

    // CLI Output applets
    foreach (CLIOutputApplet* applet, mCLIOutputAppletsList) {
        applet->activate(mCfgMan, this);
        applet->repeatingAction(mCfgMan, this);
    }
}

void Panel::setPanelUI() {
#if mPanelLayout == 0
#include <QHBoxLayout>
#else
#include <QVBoxLayout>
#endif

    // Create layout
    int margin = mCfgMan->mPanels.at(mPanelID - 1).margin;
    mPanelFrame = new QFrame();
    mPanelFrame->setObjectName("mPanelFrame");
    mPanelFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    if (mPanelLayout == Horizontal) {
        QHBoxLayout* panelLayout = new QHBoxLayout(this);
        panelLayout->setContentsMargins(0, 0, 0, 0);
        panelLayout->setSpacing(0);
        panelLayout->addWidget(mPanelFrame);

        mBoxLayout = new QHBoxLayout();
        mBoxLayout->setContentsMargins(margin, 1, margin, 1);
        mPanelFrame->setLayout(mBoxLayout);
    }
    else {
        QVBoxLayout* panelLayout = new QVBoxLayout(this);
        panelLayout->setContentsMargins(0, 0, 0, 0);
        panelLayout->addWidget(mPanelFrame);

        mBoxLayout = new QVBoxLayout(mPanelFrame);
        mBoxLayout->setContentsMargins(1, margin, 1, margin);
    }
    mSpacing = mCfgMan->mPanels.at(mPanelID - 1).spacing;
    mBoxLayout->setSpacing(mSpacing);

    // Set font
    mPanelFont.setFamily(mCfgMan->mFontFamily);
    mPanelFont.setPointSize(mCfgMan->mFontSize);
    this->setFont(mPanelFont);
    mFontMetrics = new QFontMetrics(mPanelFont);

    // Set theme
    mStylesheet = mCfgMan->mStylesheet;
    QString stylesheetPath = QString("/usr/share/plainDE/styles/%1").arg(mStylesheet);
    QFile stylesheetReader(stylesheetPath);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Set panel background image
    QString bgrPath = mCfgMan->mPanels.at(mPanelID - 1).backgroundImagePath;
    QString frameStylesheet = QString("QFrame#mPanelFrame { border-image: url(%1) "
                                      "0 0 0 0 stretch stretch; }").arg(bgrPath);
    mPanelFrame->setStyleSheet(frameStylesheet);

    // Set tooltips font
    this->setStyleSheet(this->styleSheet() + " QToolTip { font-size: " + QString::number(mPanelFont.pointSize()) + "px; }");

    // Set accent
    mAccentColor = mCfgMan->mAccent;

    // Set opacity
    mOpacity = mCfgMan->mPanels.at(mPanelID - 1).opacity;
    if (!mPanelTransparent) {
        this->setWindowOpacity(mOpacity);
    }

    // Set default icon theme
    QIcon::setThemeName(mCfgMan->mIconTheme);

    mLauncherIconSize = mCfgMan->mPanels.at(mPanelID - 1).launcherIconSize;
}

void Panel::addApplets() {
    // Applets: show applets
    /* We could use QVariantList::contains, but this approach will not save
     * order of placing applets. Using loop. */

    foreach (QVariant applet, mCfgMan->mPanels.at(mPanelID - 1).applets) {
        if (applet == "appmenu") {
            mAppMenuApplet = new AppMenuApplet(mCfgMan, this, "");
            mAppMenuApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mAppMenuApplet->mExternalWidget);
        }

        else if (applet == "spacer") {
            if (mPanelLayout == Horizontal) {
                mBoxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
            }
            else {
                mBoxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
            }
        }

        else if (applet == "splitter") {
            QLabel* splitter = new QLabel((mPanelLayout == Horizontal) ? "|" : "—");
            splitter->setAlignment(Qt::AlignCenter);
            mBoxLayout->addWidget(splitter);
            mSplitters.append(splitter);
        }

        else if (applet == "datetime") {
            mDateTimeApplet = new DateTimeApplet(mCfgMan, this, "");
            mDateTimeApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mDateTimeApplet->mExternalWidget);
        }

        else if (applet == "kblayout") {
            mKbLayoutApplet = new KbLayoutApplet(mCfgMan, this, "");
            mKbLayoutApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mKbLayoutApplet->mExternalWidget);
        }

        else if (applet == "usermenu") {
            mUserMenuApplet = new UserMenuApplet(mCfgMan, this, "");
            mUserMenuApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mUserMenuApplet->mExternalWidget);
        }

        else if (applet == "volume") {
            mVolumeApplet = new VolumeApplet(mCfgMan, this, "");
            mVolumeApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mVolumeApplet->mExternalWidget);
        }

        else if (applet == "windowlist") {
            mWinListApplet = new WindowListApplet(mCfgMan, this, "");
            mWinListApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addLayout(mWinListApplet->mExternalLayout);
        }

        else if (applet == "workspaces") {
            mWorkspacesApplet = new WorkspacesApplet(mCfgMan, this, "");
            mWorkspacesApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mWorkspacesApplet->mExternalWidget);
        }

        else if (applet == "localipv4") {
            mIPv4Applet = new LocalIPv4Applet(mCfgMan, this, "");
            mIPv4Applet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mIPv4Applet->mExternalWidget);
        }

        else if (applet == "mpris") {
            mMPRISApplet = new MPRISApplet(mCfgMan, this, "");
            mMPRISApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mMPRISApplet->mExternalWidget);
        }

        else if (applet == "battery") {
            mBatteryApplet = new BatteryApplet(mCfgMan, this, "");
            if (mBatteryApplet->mDeviceHasBattery) {
                mBatteryApplet->externalWidgetSetup(mCfgMan, this);
                mBoxLayout->addWidget(mBatteryApplet->mExternalWidget);
            }
            else {
                qDebug() << "This device does not have a battery. Deactivating Battery applet...";
            }
        }

        else if (applet.toString().startsWith("launcher:")) {
            LauncherApplet* launcher = new LauncherApplet(mCfgMan, this, applet.toString());
            launcher->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(launcher->mExternalWidget);
        }

        else if (applet == "sni") {
            mSNITrayApplet = new SNITrayApplet(mCfgMan, this, "");
            mSNITrayApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(mSNITrayApplet->mExternalWidget);
        }

        else if (applet.toString().startsWith("clioutput:")) {
            QString appletName = applet.toString().split(':')[1];
            CLIOutputApplet* cliOutputApplet = new CLIOutputApplet(mCfgMan,
                                                                   this,
                                                                   appletName);
            cliOutputApplet->externalWidgetSetup(mCfgMan, this);
            mBoxLayout->addWidget(cliOutputApplet->mExternalWidget);
            mCLIOutputAppletsList.append(cliOutputApplet);
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }
}

void Panel::setTransparency() {
    if (mStylesheet.contains("transparent")) {
        this->setAttribute(Qt::WA_TranslucentBackground);
        this->setAttribute(Qt::WA_NoSystemBackground);
        this->setAttribute(Qt::WA_TransparentForMouseEvents);
        this->setPalette(Qt::transparent);
        this->setAutoFillBackground(false);
        this->setAttribute(Qt::WA_OpaquePaintEvent);
        mPanelTransparent = true;
    }
}

void Panel::setAppletsActions() {
    // Applets: set actions
    foreach (QVariant applet, mCfgMan->mPanels.at(mPanelID - 1).applets) {
        if (applet == "appmenu") {
            mAppMenuApplet->internalWidgetSetup(mCfgMan, this);
        }

        else if (applet == "datetime") {
            mDateTimeApplet->internalWidgetSetup(mCfgMan, this);
        }

        else if (applet == "usermenu") {
            mUserMenuApplet->internalWidgetSetup(mCfgMan, this);
        }

        else if (applet == "mpris") {
            mMPRISApplet->internalWidgetSetup(mCfgMan, this);
        }
    }
}

void Panel::animation(AnimationType type) {
    if (mCfgMan->mEnableAnimation) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(this, "pos");
        panelAnimation->setDuration(250);

        unsigned short ax = 0, ay = 0;
        if (mPanelLayout == Horizontal) {
            ax = this->geometry().x();
        }
        else if (mPanelLayout == Vertical) {
            ay = this->geometry().y();
        }

        QPoint startValue, endValue;

        switch (mPanelLocation) {
        case Top:
            if (type == Show) {
                startValue = QPoint(ax, -mPanelThickness);
                endValue = QPoint(ax, 0);
            }
            else {
                startValue = QPoint(ax, 0);
                endValue = QPoint(ax, -mPanelThickness);
            }
            break;
        case Bottom:
            if (type == Show) {
                startValue = QPoint(ax, mScreenHeight);
                endValue = QPoint(ax, mScreenHeight - mPanelThickness);
            }
            else {
                startValue = QPoint(ax, mScreenHeight - mPanelThickness);
                endValue = QPoint(ax, mScreenHeight);
            }
            break;
        case Left:
            if (type == Show) {
                startValue = QPoint(-mPanelThickness, ay);
                endValue = QPoint(0, ay);
            }
            else {
                startValue = QPoint(0, ay);
                endValue = QPoint(-mPanelThickness, ay);
            }
            break;
        case Right:
            if (type == Show) {
                startValue = QPoint(mScreenWidth, ay);
                endValue = QPoint(mScreenWidth - mPanelThickness, ay);
            }
            else {
                startValue = QPoint(mScreenWidth - mPanelThickness, ay);
                endValue = QPoint(mScreenWidth, ay);
            }
            break;
        }

        panelAnimation->setStartValue(startValue);
        panelAnimation->setEndValue(endValue);
        panelAnimation->start();

        if (type == Hide) {
            this->connect(panelAnimation, &QPropertyAnimation::finished, this, [this]() {
                QThread::msleep(250);
                emit animationFinished();
            });
        }
    }
}

void Panel::highlight() {
    mPanelFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

    QTimer::singleShot(5000, Qt::CoarseTimer, this, [this]() {
        mPanelFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    });
}

void Panel::testpoint(QObject* execHolder) {
    /* This function is intended for testing your code.
     * You can use it in order to test your applet or some features.
     * It is ran in the very end of the panel initialization sequence
     * (see Panel constructor below).

     * execHolder is Initializer class instance.
     * Use it as a parent for QProcess.

     * If you want to get config.json value, use mCfgMan object fields.
     * For example:
     *     * mCfgMan->mAccent
     *     * mCfgMan->mUseTriangularTabs
     *     * etc

     * If you want to get an inner entry of current panel, use this syntax:
     *     mCfgMan->mPanels.at(mPanelID - 1)
     * For example:
     *     * mCfgMan->mPanels.at(mPanelID - 1).spacing
     *     * mCfgMan->mPanels.at(mPanelID - 1).opacity
     *     * etc */

    // here you can put your code to test

}

Panel::Panel(QObject* parent,
             ConfigManager* cfgMan,
             int id,
             QApplication* app,
             QList<Panel*> prevPanels): QWidget(nullptr) {
    qDebug() << "--------------------------------------";
    qDebug() << "Panel " << id;

    mApplication = app;
    mCfgMan = cfgMan;

    mPanelID = id;
    mExecHolder = parent;

    // we need this property to avoid interference of two panels
    // (i.e., left and top)
    mPrevPanels = prevPanels;

    mMprisCards = new QList<QWidget*>;

    this->setWindowTitle("plainPanel");
    this->setObjectName("panel");

    setPanelFlags();
    setPanelGeometry();
    setPanelUI();
    mPanelPID = app->applicationPid();
    qDebug() << "DEBUG";
    addApplets();
    setTransparency();
    mPanelWId = this->winId();
    qDebug() << "Panel PID" << mPanelPID;
    this->show();
    setPanelGeometry();
    setAppletsActions();
    setRepeatingActions();
    animation(Show);

    this->connect(static_cast<UserMenuApplet*>(mUserMenuApplet),
                  &UserMenuApplet::panelShouldQuit, this, [this]() {
        foreach (QProcess* process, mProcesses) {
            if (process != NULL) {
                delete process;
            }
        }
        mProcesses.clear();

        animation(Hide);
    });

    testpoint(mExecHolder);
}


Panel::~Panel() {
    const QJsonArray* applets = &mCfgMan->mPanels.at(mPanelID - 1).applets;
    if (applets->contains("appmenu")) {
        delete mAppMenuApplet;
    }
    if (applets->contains("datetime")) {
        delete mDateTimeApplet;
    }
    if (applets->contains("kblayout")) {
        delete mKbLayoutApplet;
    }
    if (applets->contains("usermenu")) {
        delete mUserMenuApplet;
    }
    if (applets->contains("volume")) {
        delete mVolumeApplet;
    }
    if (applets->contains("workspaces")) {
        delete mWorkspacesApplet;
    }
    if (applets->contains("localipv4")) {
        delete mIPv4Applet;
    }
    if (applets->contains("battery")) {
        delete mBatteryApplet;
    }
    if (applets->contains("mpris")) {
        delete mMPRISApplet;
    }
    if (applets->contains("sni")) {
        delete mSNITrayApplet;
    }
    if (applets->contains("windowlist")) {
        delete mWinListApplet;
    }

    foreach (CLIOutputApplet* object, mCLIOutputAppletsList) {
        delete object;
    }
    mCLIOutputAppletsList.clear();

    this->hide();

    qDebug() << "Panel" << mPanelID << "destructed.";
}
