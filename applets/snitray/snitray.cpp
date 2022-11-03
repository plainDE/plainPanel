#include "snitray.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

SNITrayApplet::SNITrayApplet(QObject* parent) : QDBusAbstractAdaptor(parent) {

}

SNITrayApplet::~SNITrayApplet() {

}

void SNITrayApplet::RegisterStatusNotifierItem(QString service) {
    qDebug() << service << "registered successfully.";
}

void SNITrayApplet::RegisterStatusNotifierHost(QString service) {

}
