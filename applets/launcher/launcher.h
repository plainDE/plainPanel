#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "panel.h"
#include <QPushButton>
#include <QString>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QIcon>
#include <QPropertyAnimation>

class Launcher : public QPushButton {
public:
    Launcher(Panel* parentPanel,
             QString entry,
             int iconSz,
             QString iconTheme,
             QList<QProcess*>* processes,
             QObject* execHolder,
             bool animate);
    ~Launcher();
};

#endif // LAUNCHER_H
