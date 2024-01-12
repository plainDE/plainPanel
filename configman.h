#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonArray>
#include <QFont>

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

enum VolumeAdjustMethod {
    ALSA = 0,
    PulseAudio = 1
};

enum PanelLocation {
    Top = 1,
    Bottom = 2,
    Left = 3,
    Right = 4
};

struct PanelCfg {
    bool isNull = false;
    QJsonArray applets;
    QString screen;
    QString backgroundImagePath;
    int shift;
    double opacity;
    int margin;
    int spacing;
    qint8 thickness;
    qint8 launcherIconSize;
    PanelLocation location;
    bool expand;
};

class ConfigManager {
public:
    ConfigManager();
    void readConfig();
    void setFields();

    QJsonObject mConfigObj;

    // General settings
    QString mAccent;
    QString mAvatar;
    QString mIconTheme;
    QString mStylesheet;
    bool mTransparent;
    int mCountPanels;
    int mSecondsUntilPowerOff;
    bool mEnableAnimation;

    // Font
    QString mFontFamily;
    int mFontSize;
    QFont mFont;

    // App Menu applet settings
    QJsonArray mFavApps;
    QString mMenuIcon;
    QString mMenuText;
    int mMenuIconSize;
    bool mUseTriangularTabs;

    // Date & Time applet settings
    QString mDateFormat;
    QString mTimeFormat;
    Qt::DayOfWeek mFirstDayOfWeek;
    bool mShowDate;
    bool mShowWeekNumbers;

    // Volume applet settings
    bool mEnableOveramplification;
    VolumeAdjustMethod mVolumeAdjMethod;
    int mDefaultVolume;

    // Keyboard Layout applet settings
    QString mKbToggleMethod;
    QString mKbLayouts;
    bool mUseCountryFlag;

    // Window List applet settings
    int mWinListIconSize;
    bool mWinListShowTitles;

    // Workspaces applet settings
    bool mShowDesktopNames;

    // IPv4 applet settings
    QString mNetworkInterface;
    QString mIPAddrColor;

    // Panels
    QList<PanelCfg> mPanels;
};

#endif // CONFIGMANAGER_H
