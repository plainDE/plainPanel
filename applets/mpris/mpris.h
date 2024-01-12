#ifndef MPRISAPPLET_H
#define MPRISAPPLET_H

#include "../../applet.h"

#include <QWidget>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLabel>
#include <QNetworkReply>
#include <QVariantMap>
#include <QtConcurrent>

class MPRISApplet : public Applet {
    Q_OBJECT
public:
    MPRISApplet(ConfigManager* cfgMan,
                Panel* parentPanel,
                QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void internalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~MPRISApplet();

    QPushButton* mExternalWidget;
    QWidget* mInternalWidget;

private:
    QWidget* createPlayerCard(ConfigManager* cfgMan, QString serviceName);
    void addCards(ConfigManager* cfgMan);
    void setSize(ConfigManager* cfgMan, Panel* parentPanel);
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
