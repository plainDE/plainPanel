#ifndef APPLET_H
#define APPLET_H

#include <QObject>
#include <QWidget>
#include <QScreen>

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>

#include "configman.h"
#include "panel.h"

enum AppletType {
    Static,
    Dynamic
};

class Applet : public QObject {
    Q_OBJECT
public:
    QString mAppletID;
    AppletType mAppletType;

    Applet(QString appletID,
           ConfigManager* cfgMan,
           Panel* parentPanel);

    virtual void externalWidgetSetup();
    virtual void internalWidgetSetup();

    void preliminaryInternalWidgetSetup(int width,
                                        int height,
                                        bool canBeTransparent);

    void setBlurredBackground();

    QPair<int,int> getButtonCoordinates();

    ConfigManager* mCfgMan;
    Panel* mParentPanel;

    QWidget* mExternalWidget;
    QWidget* mInternalWidget;
};

#endif // APPLET_H
