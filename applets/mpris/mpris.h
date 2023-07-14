#ifndef MPRISAPPLET_H
#define MPRISAPPLET_H

#include "../../panel.h"

#include <QWidget>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLabel>
#include <QNetworkReply>
#include <QVariantMap>
#include <QtConcurrent>

class MPRISApplet : public QWidget {
public:
    MPRISApplet(PanelLocation panelLocation,
                int panelThickness,
                int screenWidth,
                int screenHeight,
                int buttonCoord1,
                int buttonCoord2,
                QString theme,
                double opacity);
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
    void setPlayerCards(QList<QWidget*>* mprisCards,
                        QFont font,
                        QString stylesheet,
                        QString accent);
    void createUI(PanelLocation panelLocation,
                  int panelThickness,
                  int screenWidth,
                  int screenHeight,
                  int buttonCoord1,
                  int buttonCoord2,
                  QString theme,
                  double opacity);

    QList<QWidget*> mCards;

public slots:
    void downloadFinished(QNetworkReply* reply);
};

#endif // MPRISAPPLET_H
