#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "panel.h"
#include <QPushButton>
#include <QString>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QIcon>

class Launcher : public QPushButton {
public:
    Launcher(Panel* parentPanel,
             QString entry,
             int iconSize,
             QString iconTheme,
             QList<QProcess*>* processes,
             QObject* execHolder);
    ~Launcher();
};

#endif // LAUNCHER_H
