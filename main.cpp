#include "initializer.h"
#include "panel.h"

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


    /*QWidget w;
    w.setObjectName("panel");
    w.setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    w.setAttribute(Qt::WA_TranslucentBackground);
    w.setAttribute(Qt::WA_NoSystemBackground);
    w.setAttribute(Qt::WA_TransparentForMouseEvents);
    w.setGeometry(0, 0, 1152, 28);
    w.setStyleSheet("QWidget#panel { background-color: transparent; }");
    w.setPalette(Qt::transparent);
    w.setAutoFillBackground(false);
    w.setAttribute(Qt::WA_OpaquePaintEvent);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(1, 1, 1, 1);
    w.setLayout(layout);
    w.layout()->addWidget(new QPushButton("test"));
    w.show();*/

    a.exec();
}
