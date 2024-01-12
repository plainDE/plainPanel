#ifndef INITIALIZER_H
#define INITIALIZER_H

#include "applet.h"

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <KWindowSystem>
#include <QMessageBox>

class Initializer : public QObject {
    Q_CLASSINFO("D-Bus Interface", "org.plainDE.actions")
    Q_OBJECT

public:
    Initializer(QApplication* app = nullptr);
    QWidget* newPanel(qint8 number);
    void autostart();
    ~Initializer();

    QString MAJOR_VER = "0", MINOR_VER = "7", PATCH_VER = "0";
    QApplication* mApp;

public slots:
    void reconfigurePanel();
    void highlightPanel(int n);

private:
    ConfigManager* mCfgMan;
};

#endif // INITIALIZER_H
