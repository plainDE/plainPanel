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

struct dateTimeUI {
    QWidget* calendarWidget;
};

class DateTimeApplet : public Applet {
public:
    QString __getDisplayedData__(QString timeFormat, QString dateFormat);
    QString __getDisplayedData__(QString timeFormat);
    dateTimeUI __createUI__(PanelLocation location, short panelHeight, QFont font,
                            short buttonX, short buttonXRight, Qt::DayOfWeek firstDay);

    QString getCurrentTime(QString timeFormat);
    QString getCurrentDate(QString dateFormat);
    void createCalendar();
    void buildCalendar(Month month, QWidget* calendarWidget);
};

#endif // DATETIME_H
