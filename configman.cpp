#include "configman.h"

void ConfigManager::readConfig() {
    QString configPath = QDir::homePath() + "/.config/plainDE/config.json";
    if (!QFile::exists(configPath)) {
        qDebug() << "~/.config/plainDE/config.json does not exist. "
                    "Generating new config...";
        system("python3 /usr/share/plainDE/tools/genconfig.py");
    }
    else {
        system("python3 /usr/share/plainDE/tools/update-config.py");
    }

    QFile file(configPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data = file.readAll();
    file.close();
    mConfigObj = QJsonDocument::fromJson(data.toUtf8()).object();
}

void ConfigManager::setFields() {
    // General settings
    mAccent = mConfigObj["accent"].toString();
    mAvatar = mConfigObj["avatar"].toString();
    mIconTheme = mConfigObj["iconTheme"].toString();
    mStylesheet = mConfigObj["theme"].toString();
    mTransparent = mStylesheet.contains("transparent");
    mCountPanels = mConfigObj["countPanels"].toInt();
    mEnableAnimation = mConfigObj["enableAnimation"].toBool();
    mSecondsUntilPowerOff = mConfigObj["secondsUntilPowerOff"].toInt();

    // Font
    mFontFamily = mConfigObj["fontFamily"].toString();
    mFontSize = mConfigObj["fontSize"].toInt();
    mFont = QFont(mFontFamily, mFontSize);

    // App Menu applet settings
    mFavApps = mConfigObj["favApps"].toArray();
    mMenuIcon = mConfigObj["menuIcon"].toString();
    mMenuText = mConfigObj["menuText"].toString();
    mMenuIconSize = mConfigObj["menuIconSize"].toInt();
    mUseTriangularTabs = mConfigObj["useTriangularTabs"].toBool();

    // Date & Time applet settings
    mDateFormat = mConfigObj["dateFormat"].toString();
    mTimeFormat = mConfigObj["timeFormat"].toString();
    // https://code.woboq.org/qt5/qtbase/src/corelib/global/qnamespace.h.html#Qt::DayOfWeek
    mFirstDayOfWeek = static_cast<Qt::DayOfWeek>(mConfigObj["firstDayOfWeek"].toInt());
    mShowDate = mConfigObj["showDate"].toBool();
    mShowWeekNumbers = mConfigObj["showWeekNumbers"].toBool();

    // Volume applet settings
    mEnableOveramplification = mConfigObj["enableOveramplification"].toBool();
    bool pulseaudio = !mConfigObj["volumeAdjustMethod"].toString().compare("PulseAudio");
    mVolumeAdjMethod = pulseaudio ? PulseAudio : ALSA;
    mDefaultVolume = mConfigObj["defaultVolume"].toInt();

    // Keyboard Layout applet settings
    mKbToggleMethod = mConfigObj["kbLayoutToggle"].toString();
    mKbLayouts = mConfigObj["kbLayouts"].toString();
    mUseCountryFlag = mConfigObj["useCountryFlag"].toBool();

    // Window List applet settings
    mWinListIconSize = mConfigObj["winListIconSize"].toInt();
    mWinListShowTitles = mConfigObj["winListShowTitles"].toBool();

    // Workspaces applet settings
    mShowDesktopNames = mConfigObj["showDesktopNames"].toBool();

    // IPv4 applet settings
    mNetworkInterface = mConfigObj["ipIfname"].toString();
    mIPAddrColor = mConfigObj["ipColor"].toString();

    // Panels
    for (int i = 1; i <= 4; ++i) {
        QString currentPanelName = QString("panel%1").arg(QString::number(i));
        QJsonObject currentPanelObj = mConfigObj[currentPanelName].toObject();
        PanelCfg currentPanel;

        if (!mConfigObj[currentPanelName].isNull()) {
            currentPanel.applets = currentPanelObj["applets"].toArray();
            currentPanel.screen = currentPanelObj["screen"].toString();
            currentPanel.backgroundImagePath = currentPanelObj["backgroundImage"].toString();
            currentPanel.shift = currentPanelObj["shift"].toInt();
            currentPanel.opacity = currentPanelObj["opacity"].toDouble();
            currentPanel.margin = currentPanelObj["margin"].toInt();
            currentPanel.thickness = currentPanelObj["thickness"].toInt();
            currentPanel.launcherIconSize = currentPanelObj["launcherIconSize"].toInt();
            currentPanel.expand = currentPanelObj["expand"].toBool();
            currentPanel.spacing = currentPanelObj["spacing"].toInt();

            PanelLocation loc;
            QString locData = currentPanelObj["location"].toString();
            if (!locData.compare("top")) {
                loc = Top;
            }
            else if (!locData.compare("bottom")) {
                loc = Bottom;
            }
            else if (!locData.compare("left")) {
                loc = Left;
            }
            else if (!locData.compare("right")) {
                loc = Right;
            }
            else {  // Use 'top' as fallback
                loc = Top;
            }
            currentPanel.location = loc;
        }
        else {
            currentPanel.isNull = true;
        }

        mPanels.append(currentPanel);
    }
}

ConfigManager::ConfigManager() {

}
