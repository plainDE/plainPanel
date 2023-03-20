#include "../../panel.h"

#include <QCalendarWidget>

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

class DateTimeApplet : public QCalendarWidget {
public:
    QString __getDisplayedData__(QString timeFormat, QString dateFormat, PanelLayout panelLayout);
    QString __getDisplayedData__(QString timeFormat);
    void createUI(Qt::DayOfWeek firstDay,
                  PanelLocation panelLocation,
                  QFont font,
                  int panelThickness,
                  int screenWidth,
                  int screenHeight,
                  int buttonCoord1,
                  int buttonCoord2,
                  double opacity);

    QString getCurrentTime(QString timeFormat);
    QString getCurrentDate(QString dateFormat);
    DateTimeApplet(Qt::DayOfWeek firstDay,
                   PanelLocation panelLocation,
                   QFont font,
                   int panelThickness,
                   int screenWidth,
                   int screenHeight,
                   int buttonCoord1,
                   int buttonCoord2,
                   double opacity);
    ~DateTimeApplet();
};

#endif // DATETIME_H
