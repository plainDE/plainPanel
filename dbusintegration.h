#ifndef DBUSINTEGRATION_H
#define DBUSINTEGRATION_H

#include <QDBusAbstractAdaptor>


class DBusIntegration : public QDBusAbstractAdaptor {
    Q_CLASSINFO("D-Bus Interface", "org.plainDE.actions")

public:
    void init();
    DBusIntegration(QString service,
                    QString path,
                    QString interfaceName,
                    QObject* parent);

public slots:
    void test();
};

#endif // DBUSINTEGRATION_H
