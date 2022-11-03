#ifndef DBUSINTEGRATION_H
#define DBUSINTEGRATION_H

#include <QDBusAbstractAdaptor>


class DBusIntegration : public QDBusAbstractAdaptor {
public:
    void init();
    DBusIntegration(QString service,
                    QString path,
                    QString interfaceName,
                    QObject* parent);
};

#endif // DBUSINTEGRATION_H
