#include <QString>
#include <QPushButton>
#include <QIcon>
#include <QWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFont>
#include <QStringList>
#include <QDir>
#include <QSettings>
#include <QTime>
#include <QDate>
#include <QCalendar>
#include <QDebug>
#include <QFile>

#ifndef APPLET_H
#define APPLET_H

enum AppletType {
    appMenu,
    windowList,
    dateTime,
    volume,
    kbLayout
};


class Applet
{
public:
    Applet();
};

#endif // APPLET_H