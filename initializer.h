#ifndef INITIALIZER_H
#define INITIALIZER_H

#include "applet.h"

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <KWindowSystem>
#include <QMessageBox>
#include <QMediaPlayer>

class Initializer : public QObject {
    Q_CLASSINFO("D-Bus Interface", "org.plainDE.actions")
    Q_OBJECT

public:
    Initializer(QApplication* app = nullptr);
    QWidget* newPanel(qint8 number);
    void autostart();
    void playStartupSound();
    ~Initializer();

    QString MAJOR_VER = "0", MINOR_VER = "7", PATCH_VER = "0";
    QApplication* mApp;

public slots:
    void reconfigurePanel();
    void highlightPanel(int n);

signals:
    void panelShouldQuit();

private:
    ConfigManager* mCfgMan;
    QMediaPlayer* mPlayer;
};

#endif // INITIALIZER_H
