#include "panel.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    panel w;
    w.show();
    return a.exec();
}
