#include "../../applet.h"
#include "../../panel.h"

#ifndef DATETIME_H
#define DATETIME_H

enum Month {
    Jan = 1,
    Feb,
    Mar,
    Apr,
    May,
    Jun,
    Jul,
    Aug,
    Sep,
    Oct,
    Nov,
    Dec
};

class DateTimeApplet : public Applet {
public:
    QString __appletName__ = "Date & time";
    AppletType __appletType__ = dateTime;
    QString __getDisplayedData__(QString timeFormat, QString dateFormat);
    QString __getDisplayedData__(QString timeFormat);
    QWidget* __createUI__(PanelLocation location, short panelHeight, QFont font, short buttonX);
    bool __needsToBeUpdated__ = true;

    QString getCurrentTime(QString timeFormat);
    QString getCurrentDate(QString dateFormat);
    void createCalendar();
    void buildCalendar(Month month, QWidget* calendarWidget);


};

#endif // DATETIME_H