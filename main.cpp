#include "initializer.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

QJsonObject config;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    if (QString::compare(getenv("XDG_SESSION_TYPE"), "x11", Qt::CaseInsensitive) != 0) {
        qDebug() << "plainPanel currently works on X11 only. Quitting...";
        exit(0);
    }

    Initializer* init = new Initializer(&a);

    a.exec();
}
