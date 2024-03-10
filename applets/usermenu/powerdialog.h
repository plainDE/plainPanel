#ifndef POWERDIALOG_H
#define POWERDIALOG_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QString>
#include <QHash>
#include <QTimer>
#include <QMediaPlayer>


#include "../../panel.h"
#include "../../configman.h"

enum Type {
    PowerOff,
    Reboot,
    LogOut
};

class PowerDialog : public QObject {
    Q_OBJECT
public:
    PowerDialog(Type type,
                Panel* parentPanel,
                ConfigManager* cfgMan);
    QString getInfoText();
    void startTimer();
    void setTransparency(QWidget* widget,
                         int ax,
                         int ay,
                         int width,
                         int height);

    Panel* mParentPanel;
    int mSecondsLeft;
    Type mType;
    QHash<Type,QString> mIconByType;
    QHash<Type,QString> mStringByType;
    QHash<Type,QString> mActionByType;
    QTimer* mTimer;
    QMediaPlayer* mPlayer;
    bool mPlayLogoutSound = false;

signals:
    void actionRequested();
    void cancelled();
};

#endif // POWERDIALOG_H
