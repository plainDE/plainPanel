#ifndef PANEL_H
#define PANEL_H

#include "initializer.h"

#include <QWidget>
#include <QBoxLayout>
#include <QGuiApplication>
#include <QApplication>
#include <QScreen>
#include <QDBusAbstractAdaptor>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QPropertyAnimation>
#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QDial>
#include <QBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QtConcurrent>
#include <KWindowSystem>


QT_BEGIN_NAMESPACE
namespace Ui { class panel; }
QT_END_NAMESPACE

enum PanelLocation {
    Top = 1,
    Bottom = 2,
    Left = 3,
    Right = 4
};

enum PanelInterference {
    None = 0,
    TopAndLeft = 1,
    TopAndRight = 2,
    BottomAndLeft = 3,
    BottomAndRight = 4,
    LeftAndTop = 5,
    LeftAndBottom = 6,
    RightAndTop = 7,
    RightAndBottom = 8,
    Match = 9
};

enum PanelLayout {
    Horizontal = 1,
    Vertical = 2
};

enum AnimationType {
    Show = 1,
    Hide = 2
};

class Panel : public QWidget {
    Q_OBJECT


public:
    Panel(QObject *parent = nullptr,
          QJsonObject* config = nullptr,
          int id = 0,
          QApplication* app = nullptr,
          QList<Panel*>* prevPanels = nullptr);
    void animation(AnimationType type);
    void updateWinList();           // Window List applet
    void updateWinList(bool);       // Window List applet (without titles, for vertical panel)
    ~Panel();



private:
    QJsonValue getConfigValue(QString key);
    QJsonValue getConfigValue(QString section, QString key);
    void basicInit(QJsonObject* config, qint8 number);
    void setRepeatingActions();
    void readConfig();
    void setPanelFlags();
    PanelInterference checkPanelsInterference(PanelLocation loc1, PanelLocation loc2);
    void setPanelGeometry();
    void shortenTitle(QString* src, QPushButton* button);
    void setPanelUI();
    void addApplets();
    void testpoint(QObject* parent);
    void freeUnusedMemory(bool quit);

    QApplication *mApplication;
    QJsonObject* mConfig;
    WId mPanelWId;
    pid_t mPanelPID;
    QString mPanelName;
    QObject* mExecHolder;

    QScreen* mPrimaryScreen;
    int mScreenWidth, mScreenHeight;
    QList<Panel*>* mPrevPanels;

    int mPanelThickness;
    PanelLayout mPanelLayout;
    int mPanelWidth, mPanelHeight;
    PanelLocation mPanelLocation;
    int mAxisShift;

    QBoxLayout* mBoxLayout;
    QVariantList mActiveAppletsList;
    QHash<QString,QWidget*> mAppletWidgets;
    QList<QLabel*> mSplitters;
    QList<QTimer*> mActiveTimers;

    QFont mPanelFont;
    QFontMetrics* mFontMetrics;
    QString mStylesheet;
    QString mAccentColor;
    double mOpacity;

    bool mShowDate;
    QString mTimeFormat, mDateFormat;

    QBoxLayout* mWindowListLayout;
    QHash<WId,QString> titleByWId;

    QList<QWidget*>* mMprisCards;

    int mIPAngle;

    QHash<QString,QObject*> mOtherItems;

    QString mIfname;
    QString mLastIP;

    QString mBatteryName;
    qint8 mBatteryIconSize;

    QList<QProcess*> mProcesses;

    QBoxLayout* mSNILayout;
    QHash<QString,QPushButton*> mSniWidgets;

signals:
    void animationFinished();
    void winListUpdateRequired();

public slots:
    void toggleAppMenu();
    void toggleCalendar();
    void toggleUserMenu();
    void toggleMPRIS();

private slots:
    // updating applets data
    void setVolume();               // Volume applet
    void accentActiveWindow();      // Window List applet
    void updateDateTime(bool);      // Date & Time applet (Time + Date)
    void updateDateTime();          // Date & Time applet (Time only)
    void updateKbLayout();          // Keyboard layout applet (ISO code)
    void updateKbLayout(bool);      // Keyboard layout applet (flag)
    void updateWinTitles();         // Window List applet
    void updateWorkspaces();        // Workspaces applet
    void updateLocalIPv4();         // Local IP applet
    void updateBatteryState();      // Battery applet
    void updateBatteryStateDark();  // Battery applet (on dark theme)
};
#endif // PANEL_H
