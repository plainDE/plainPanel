#include "clock.h"

#include <QString>
#include <QTime>

QString clockApplet::getCurrentTime(bool showSeconds) {
    if (!showSeconds) {
        return QTime::currentTime().toString("hh:mm");
    }
    else {
        return QTime::currentTime().toString("hh:mm:ss");
    }
}
