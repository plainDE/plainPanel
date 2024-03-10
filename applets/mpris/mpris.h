#ifndef MPRISAPPLET_H
#define MPRISAPPLET_H

#include "../../staticapplet.h"

#include <QWidget>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLabel>
#include <QNetworkReply>
#include <QVariantMap>
#include <QtConcurrent>

class MPRISApplet : public StaticApplet {
    Q_OBJECT
public:
    MPRISApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void internalWidgetSetup() override;
    ~MPRISApplet();

private:
    QWidget* createPlayerCard(QString serviceName);
    void addCards();
    void setSize();
    void updateIdentity(QString serviceName,
                        QLabel* iconLabel,
                        QLabel* titleLabel);
    QVariantMap getMetadata(QString playerService);
    QString getProperty(QString playerService,
                        QString playerPath,
                        QString propertyInterface,
                        QString propertyName);
    void playPause(QString serviceName,
                   QLabel* iconLabel,
                   QLabel* titleLabel);
    void previous(QString serviceName,
                  QLabel* iconLabel,
                  QLabel* titleLabel);
    void next(QString serviceName,
              QLabel* iconLabel,
              QLabel* titleLabel);

    QDBusMessage mRequest, mResponse;
    QList<QWidget*> mCards;
    bool mHasCards;
    bool mDestructed;

signals:
    void readyToDestroy();

private slots:
    void destructCards();
};

#endif // MPRISAPPLET_H
