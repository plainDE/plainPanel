#ifndef STATUSNOTIFIERWATCHER_H
#define STATUSNOTIFIERWATCHER_H

#include <QObject>
#include <QStringList>
#include <QIcon>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QPixmap>
#include <QDBusServiceWatcher>

struct QXdgDBusImageVector {
    int width;
    int height;
    QByteArray data;
};

class StatusNotifierWatcher : public QObject {
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")
    Q_OBJECT

public:
    explicit StatusNotifierWatcher(QObject *parent = nullptr);

    Q_PROPERTY(bool IsStatusNotifierHostRegistered READ isStatusNotifierHostRegistered)
    inline bool isStatusNotifierHostRegistered() const
    { return !hosts.empty(); }

    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ registeredStatusNotifierItems)
    inline QStringList registeredStatusNotifierItems() const
    { return items; }

public slots:
    void RegisterStatusNotifierItem(const QString &serviceOrPath);
    void RegisterStatusNotifierHost(const QString &service);
    void UnregisterStatusNotifierItem(const QString &service, const QString &path);

signals:
    void StatusNotifierItemRegistered(const QString &service);
    void StatusNotifierItemUnregistered(const QString &service);
    void StatusNotifierHostRegistered();

public:
    QStringList deletedItems;

private:
    QStringList hosts;
    QStringList items;
    QList<QDBusServiceWatcher*> watchers;
};

#endif // STATUSNOTIFIERWATCHER_H
