#ifndef PANEL_H
#define PANEL_H


#include "configman.h"

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
#include <QJsonDocument>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
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
#include <QBitmap>


QT_BEGIN_NAMESPACE
namespace Ui { class panel; }
QT_END_NAMESPACE

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
    Horizontal = 0,
    Vertical = 1
};

enum AnimationType {
    Show = 1,
    Hide = 2
};

class Panel : public QWidget {
    Q_OBJECT

public:
    Panel(QObject *parent = nullptr,
          ConfigManager* cfgMan = nullptr,
          int id = 0,
          QApplication* app = nullptr,
          QList<Panel*> prevPanels = {});
    void animation(AnimationType type);
    void updateWinList();           // Window List applet
    void updateWinList(bool);       // Window List applet (without titles, for vertical panel)
    ~Panel();

    void setRepeatingActions();
    void setPanelFlags();
    void setPanelGeometry();
    void setPanelUI();
    void addApplets();
    void setTransparency();
    void setAppletsActions();
    void testpoint(QObject* parent);

    void highlight();

    int getWindowDataSize(QString title);
    QString shortenWindowData(QPushButton* button, QString title);

    QApplication *mApplication;
    ConfigManager* mCfgMan;
    WId mPanelWId;
    pid_t mPanelPID;
    int mPanelID;
    QObject* mExecHolder;

    QScreen* mPanelScreen = nullptr;
    QRect mScreenGeometry;
    int mScreenWidth, mScreenHeight;
    QList<Panel*> mPrevPanels;

    int mPanelThickness;
    int mSpacing;
    PanelLayout mPanelLayout;
    int mPanelWidth, mPanelHeight;
    PanelLocation mPanelLocation;
    int mAxisShift;
    QRect mTargetGeometry;

    QBoxLayout* mBoxLayout;
    QVariantList mActiveAppletsList;
    QHash<QString,QWidget*> mAppletWidgets;
    QList<QLabel*> mSplitters;
    QList<QTimer*> mActiveTimers;

    QFont mPanelFont;
    QFontMetrics* mFontMetrics;
    QString mStylesheet;
    bool mPanelTransparent = false;
    QString mAccentColor;
    double mOpacity;

    bool mShowDate;
    QString mTimeFormat, mDateFormat;

    QBoxLayout* mWindowListLayout;

    int mLauncherIconSize;

    QList<QWidget*>* mMprisCards;

    int mIPAngle;

    QHash<QString,QObject*> mOtherItems;

    qint8 mCountWorkspaces;

    QList<QProcess*> mProcesses;

    QBoxLayout* mSNILayout;

    bool mHasCLIOutputApplet = false;

private:
    QFrame* mPanelFrame;

signals:
    void animationFinished();
};
#endif // PANEL_H
