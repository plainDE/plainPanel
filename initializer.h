#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <KX11Extras>

class Initializer : public QObject {
    Q_CLASSINFO("D-Bus Interface", "org.plainDE.actions");
    Q_OBJECT

public:
    Initializer();
    void readConfig();
    QWidget* newPanel(qint8 number);
    void autostart();
    ~Initializer();


public slots:
    void reconfigurePanel();
};

#endif // INITIALIZER_H
