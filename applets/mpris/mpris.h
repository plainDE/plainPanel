#ifndef MPRISAPPLET_H
#define MPRISAPPLET_H

#include "../../panel.h"

#include <QWidget>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLabel>
#include <QNetworkReply>
#include <QVariantMap>

class MPRISApplet {
public:
    MPRISApplet();
    QVariantMap getMetadata(QString playerService);
    QString getProperty(QString playerService,
                        QString playerPath,
                        QString propertyInterface,
                        QString propertyName);

    void updateIdentity(QString serviceName,
                        QLabel* iconLabel,
                        QLabel* titleLabel);

    void playPauseMedia(QString serviceName,
                        QLabel* iconLabel,
                        QLabel* titleLabel);
    void previous(QString serviceName,
                  QLabel* iconLabel,
                  QLabel* titleLabel);
    void next(QString serviceName,
              QLabel* iconLabel,
              QLabel* titleLabel);

    QWidget* createPlayerCard(QString serviceName,
                              QFont font,
                              QString theme,
                              QString accent);

    QWidget* createUI(PanelLocation location,
                      short panelHeight,
                      QFont font,
                      short buttonX,
                      short buttonXRight,
                      QString theme,
                      qreal opacity,
                      QString accent);

public slots:
    void downloadFinished(QNetworkReply* reply);
};

#endif // MPRISAPPLET_H
