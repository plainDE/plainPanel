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

class Applet : public QObject {
    Q_OBJECT
public:
    Applet(ConfigManager* cfgMan,
           Panel* parentPanel,
           QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void internalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);

    void setBlurredBackground(QWidget* internalWidget);
    QPair<int,int> getButtonCoordinates(QWidget* externalWidget,
                                        Panel* parentPanel);
    void preliminaryInternalWidgetSetup(QWidget* internalWidget,
                                        QWidget* externalWidget,
                                        ConfigManager* cfgMan,
                                        Panel* parentPanel,
                                        int width,
                                        int height,
                                        bool canBeTransparent);

    QWidget* mExternalWidget;
    QWidget* mInternalWidget;
};

#endif // APPLET_H
