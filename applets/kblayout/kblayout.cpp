#include "kblayout.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

void KbLayoutApplet::externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel) {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(cfgMan->mFont);
    mExternalWidget->setObjectName("kbLayoutButton");

    int buttonWidth = parentPanel->mFontMetrics->horizontalAdvance("AA");
    if (parentPanel->mPanelLayout == Horizontal) {
        mExternalWidget->setMaximumWidth(buttonWidth);
    }
    else {
        mExternalWidget->setMaximumHeight(parentPanel->mFontMetrics->height());
    }

    mExternalWidget->setFlat(true);
    int iconSize = parentPanel->mPanelThickness / 2 + 2;
    mExternalWidget->setIconSize(QSize(iconSize, iconSize));
}

void KbLayoutApplet::repeatingAction(ConfigManager*, Panel*, bool) {
    QString code = getCurrentLayoutISOCode();
    mExternalWidget->setIcon(mFlagByCode[code]);
    mExternalWidget->setToolTip(code);
}

void KbLayoutApplet::repeatingAction(ConfigManager*, Panel*) {
    QString code = getCurrentLayoutISOCode();
    mExternalWidget->setText(code);
}

void KbLayoutApplet::activate(ConfigManager* cfgMan, Panel* parentPanel) {
    mInterval = 350;
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    if (cfgMan->mUseCountryFlag) {
        connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel, true);
        });
    }
    else {
        connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel);
        });
    }
    mTimer->start();
}

void KbLayoutApplet::setLayouts(ConfigManager* cfgMan, Panel* parentPanel) {
    // Using setxkbmap to set keyboard layouts and toggle method
    QString layouts = cfgMan->mKbLayouts;
    QString toggleMethod = cfgMan->mKbToggleMethod;

    if (!layouts.isEmpty() && !toggleMethod.isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(parentPanel->mExecHolder);
        setxkbmapProcess->start("setxkbmap", {"-layout",
                                              layouts,
                                              "-option",
                                              toggleMethod});
    }
    else {
        qDebug() << "Incorrect format of kbLayouts or kbToggleMethod "
                    "config entry. Couldn't run setxkbmap. Check these "
                    "entries in ~/.config/plainDE/config.json file.";
    }
}

void KbLayoutApplet::connectToXServer() {
    mKbDisplay = XkbOpenDisplay(getenv("DISPLAY"), NULL, NULL, NULL, NULL, &mDisplayResult);
    mKeyboard = XkbAllocKeyboard();
}

void KbLayoutApplet::setISOCodes() {
    QFile file("/usr/share/plainDE/layouts.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data = file.readAll();
    file.close();
    mLayoutCodes = QJsonDocument::fromJson(data.toUtf8()).object();
}

void KbLayoutApplet::cacheFlagIcons(ConfigManager* cfgMan) {
    foreach (QString layout, cfgMan->mKbLayouts.split(',')) {
        mFlagByCode[layout] = QIcon("/usr/share/flags/" + layout + ".png");
    }
}

KbLayoutApplet::KbLayoutApplet(ConfigManager* cfgMan,
                               Panel* parentPanel,
                               QString additionalInfo) : Applet(cfgMan, parentPanel, additionalInfo) {
    setLayouts(cfgMan, parentPanel);
    setISOCodes();
    cacheFlagIcons(cfgMan);
    connectToXServer();
}

QString KbLayoutApplet::getCurrentLayoutISOCode() {
    // Obtain symbolic names from the server
    XkbGetNames(mKbDisplay, XkbGroupNamesMask, mKeyboard);
    XkbGetState(mKbDisplay, XkbUseCoreKbd, &mState);

    // Get language code
    mLayout = XGetAtomName(mKbDisplay, mKeyboard->names->groups[mState.group]);

    // Returning converted ISO code
    return mLayoutCodes[mLayout].toString();
}

KbLayoutApplet::~KbLayoutApplet() {

}
