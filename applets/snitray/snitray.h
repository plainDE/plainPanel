#ifndef SNITRAYAPPLET_H
#define SNITRAYAPPLET_H

#include <QObject>
#include <QDBusAbstractAdaptor>

class SNITrayApplet : public QDBusAbstractAdaptor {
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")
    Q_OBJECT

public:
    SNITrayApplet(QObject* parent);
    ~SNITrayApplet();

public slots:
    void RegisterStatusNotifierItem(QString service);
    void RegisterStatusNotifierHost(QString service);

public:
    int testProperty;
};

#endif // SNITRAYAPPLET_H
