#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <KWindowSystem>
#include <QMessageBox>

class Initializer : public QObject {
    Q_CLASSINFO("D-Bus Interface", "org.plainDE.actions");
    Q_OBJECT

public:
    Initializer(QApplication* app = nullptr);
    void readConfig();
    void setxkbmap();
    QWidget* newPanel(qint8 number);
    void autostart();
    ~Initializer();

    QApplication* mApp;

public slots:
    void reconfigurePanel();
};

#endif // INITIALIZER_H
