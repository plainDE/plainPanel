#include "panel.h"
#include "initializer.h"

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
#include "applets/splitter/splitter.h"
#include "applets/spacer/spacer.h"

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

    if (!mCfgMan->mPanels.at(mPanelID - 1).enableAutoHide) {
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
    }

    KWindowSystem::setOnAllDesktops(mPanelWId, true);
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
        KWindowSystem::setOnDesktop(mPanelWId, KWindowSystem::currentDesktop());
    });
    mCountWorkspaces = KWindowSystem::numberOfDesktops();
}

void Panel::setOnCenter() {
    if (mPanelLayout == Horizontal) {
        int panelWidth = this->width();
        int screenWidth = mScreenGeometry.width();
        int relativeX = (screenWidth - panelWidth) / 2;
        int absoluteX = mScreenGeometry.x() + relativeX;
        this->move(absoluteX, this->y());
    }
    else {
        int panelHeight = this->height();
        int screenHeight = mScreenGeometry.height();
        int relativeY = (screenHeight - panelHeight) / 2;
        int absoluteY = mScreenGeometry.y() + relativeY;
        this->move(this->x(), absoluteY);
    }
}

void Panel::setRepeatingActions() {
    // here we bring to life QTimers for updating applets data

    foreach (QObject* applet, mAppletList) {
        Applet* appletObj = static_cast<Applet*>(applet);
        if (appletObj->mAppletType == Dynamic) {
            static_cast<DynamicApplet*>(appletObj)->activate();
        }
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
    // Initializing applets
    foreach (QVariant applet, mCfgMan->mPanels.at(mPanelID - 1).applets) {
        QString appletName = applet.toString();
        qDebug() << appletName;

        if (!appletName.compare("appmenu")) {  // org.plainDE.appMenu
            Applet* applet = new AppMenuApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("sni")) {  // org.plainDE.sniTray
            Applet* applet = new SNITrayApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("battery")) {  // org.plainDE.battery
            if (BatteryApplet::deviceHasBattery()) {
                Applet* applet = new BatteryApplet(mCfgMan, this);
                mAppletList.append(applet);
            }
            else {
                qDebug() << "This device does not have a battery. "
                            "Deactivating Battery applet...";
            }
        }

        else if (!appletName.compare("mpris")) {  // org.plainDE.mpris
            Applet* applet = new MPRISApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("volume")) {  // org.plainDE.volume
            Applet* applet = new VolumeApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("kblayout")) {  // org.plainDE.kbLayout
            Applet* applet = new KbLayoutApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("datetime")) {  // org.plainDE.dateTime
            Applet* applet = new DateTimeApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("usermenu")) {  // org.plainDE.userMenu
            Applet* applet = new UserMenuApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("windowlist")) {  // org.plainDE.windowList
            Applet* applet = new WindowListApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("localipv4")) {  // org.plainDE.localIPv4
            Applet* applet = new LocalIPv4Applet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("workspaces")) {  // org.plainDE.workspaces
            Applet* applet = new WorkspacesApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (appletName.startsWith("launcher:")) {  // org.plainDE.launcher
            Applet* applet = new LauncherApplet(mCfgMan, this, appletName);
            mAppletList.append(applet);
        }

        else if (appletName.startsWith("clioutput:")) {  // org.plainDE.cliOutput
            QString name = appletName.split(':')[1];
            Applet* applet = new CLIOutputApplet(mCfgMan, this, name);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("splitter")) {  // org.plainDE.splitter
            Applet* applet = new SplitterApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else if (!appletName.compare("spacer")) {
            Applet* applet = new SpacerApplet(mCfgMan, this);
            mAppletList.append(applet);
        }

        else {
            qDebug() << "Unknown applet:" << applet;
        }
    }

    // Adding applets to panel
    foreach (QObject* applet, mAppletList) {
        Applet* appletObj = static_cast<Applet*>(applet);
        appletObj->externalWidgetSetup();
        mBoxLayout->addWidget(appletObj->mExternalWidget);
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
    foreach (QObject* applet, mAppletList) {
        static_cast<Applet*>(applet)->internalWidgetSetup();
    }
}

void Panel::animation(AnimationType type) {
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
        else if (type == Hide) {
            startValue = QPoint(ax, 0);
            endValue = QPoint(ax, -mPanelThickness);
        }
        else {
            startValue = QPoint(ax, 0);
            endValue = QPoint(ax, -mPanelThickness + 2);
            mAutoHidden = true;
        }
        break;
    case Bottom:
        if (type == Show) {
            startValue = QPoint(ax, mScreenHeight);
            endValue = QPoint(ax, mScreenHeight - mPanelThickness);
        }
        else if (type == Hide) {
            startValue = QPoint(ax, mScreenHeight - mPanelThickness);
            endValue = QPoint(ax, mScreenHeight);
        }
        else {
            startValue = QPoint(ax, mScreenHeight - mPanelThickness);
            endValue = QPoint(ax, mScreenHeight - 2);
            mAutoHidden = true;
        }
        break;
    case Left:
        if (type == Show) {
            startValue = QPoint(-mPanelThickness, ay);
            endValue = QPoint(0, ay);
        }
        else if (type == Hide) {
            startValue = QPoint(0, ay);
            endValue = QPoint(-mPanelThickness, ay);
        }
        else {
            startValue = QPoint(0, ay);
            endValue = QPoint(-mPanelThickness + 2, ay);
            mAutoHidden = true;
        }
        break;
    case Right:
        if (type == Show) {
            startValue = QPoint(mScreenWidth, ay);
            endValue = QPoint(mScreenWidth - mPanelThickness, ay);
        }
        else if (type == Hide) {
            startValue = QPoint(mScreenWidth - mPanelThickness, ay);
            endValue = QPoint(mScreenWidth, ay);
        }
        else {
            startValue = QPoint(mScreenWidth - mPanelThickness, ay);
            endValue = QPoint(mScreenWidth - 2, ay);
            mAutoHidden = true;
        }
        break;
    }


    if (mCfgMan->mEnableAnimation) {
        QPropertyAnimation* panelAnimation = new QPropertyAnimation(this, "pos");
        panelAnimation->setDuration(250);
        panelAnimation->setStartValue(startValue);
        panelAnimation->setEndValue(endValue);
        panelAnimation->start();

        this->connect(panelAnimation, &QPropertyAnimation::finished, this, [this]() {
            QThread::msleep(250);
            emit animationFinished();
        });
    }
    else {
        this->move(endValue.x(), endValue.y());
        emit animationFinished();
    }
}

void Panel::autoHideSetup() {
    if (mCfgMan->mPanels.at(mPanelID - 1).enableAutoHide) {
        setAttribute(Qt::WA_Hover, true);
        mAutoHideTimer = new QTimer();
        mAutoHideTimer->setInterval(
            mCfgMan->mPanels.at(mPanelID - 1).autoHideInterval);
        mAutoHideTimer->setSingleShot(true);
        connect(mAutoHideTimer, &QTimer::timeout, this, [this]() {
            animation(AutoHide);
        });

        if (mCfgMan->mEnableAnimation) {
            connect(this, &Panel::animationFinished, this, [this]() {
                if (!mEnableAutoHide) {
                    mAutoHideTimer->start();
                    mEnableAutoHide = true;
                }
            });
        }
        else {
            mAutoHideTimer->start();
            mEnableAutoHide = true;
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
    addApplets();
    setTransparency();
    mPanelWId = this->winId();
    this->show();
    setPanelGeometry();
    setAppletsActions();
    setRepeatingActions();
    if (mCfgMan->mPanels.at(mPanelID - 1).setOnCenter) {
        setOnCenter();
    }
    animation(Show);
    autoHideSetup();

    this->connect(static_cast<Initializer*>(mExecHolder),
                  &Initializer::panelShouldQuit, this, [this]() {
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

void Panel::enterEvent(QEvent *event) {
    if (mEnableAutoHide) {
        mAutoHideTimer->stop();
        if (mAutoHidden) {
            animation(Show);
            mAutoHidden = false;
        }
    }
    event->accept();
}

void Panel::leaveEvent(QEvent *event) {
    if (mEnableAutoHide) {
        QRect geometry = this->geometry();
        if (mScreenGeometry.contains(geometry) &&
            !geometry.contains(mapFromGlobal(QCursor::pos()))) {
            mAutoHideTimer->start();
        }
    }
    event->accept();
}

void Panel::resizeEvent(QResizeEvent* event) {
    if (mCfgMan->mPanels.at(mPanelID - 1).setOnCenter) {
        setOnCenter();
    }
    event->accept();
}

Panel::~Panel() {
    foreach (QObject* applet, mAppletList) {
        delete applet;
    }

    this->hide();
    qDebug() << "Panel" << mPanelID << "destructed.";
}
