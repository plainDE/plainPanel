#include "datetime.h"
#include "../../panel.h"


dateTimeUI DateTimeApplet::__createUI__(PanelLocation location, short panelHeight, QFont font,
                                      short buttonX, short buttonXRight, Qt::DayOfWeek firstDay) {
    QWidget* calendarWidget = new QWidget;

    // Window flags
    calendarWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                   Qt::X11BypassWindowManagerHint);

    // Geometry
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    short calendarWidth = 300;
    short calendarHeight = 200;
    short ax = 0, ay = 0;
    if (location == top) {
        ay = panelHeight + 5;
    }
    else {
        ay = primaryScreen->geometry().height() - panelHeight - calendarHeight - 5;
    }
    if (primaryScreen->geometry().width() - buttonX >= calendarWidth) {
        ax = buttonX;
    }
    else {
        ax = buttonXRight - calendarWidth;
    }
    calendarWidget->setFixedSize(calendarWidth, calendarHeight);
    calendarWidget->move(ax, ay);

    // Set font
    calendarWidget->setFont(font);


    // UI
    QVBoxLayout* calendarWidgetLayout = new QVBoxLayout;
    calendarWidgetLayout->setContentsMargins(1, 1, 1, 1);
    calendarWidget->setLayout(calendarWidgetLayout);

    QCalendarWidget* calendar = new QCalendarWidget;
    calendar->setGridVisible(true);
    calendar->setFirstDayOfWeek(firstDay);
    //calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarWidget->layout()->addWidget(calendar);


    return {calendarWidget};
}


QString DateTimeApplet::getCurrentTime(QString timeFormat) {
    return QTime::currentTime().toString(timeFormat);
}

QString DateTimeApplet::getCurrentDate(QString dateFormat) {
    return QDate::currentDate().toString(dateFormat);
}

QString DateTimeApplet::__getDisplayedData__(QString timeFormat, QString dateFormat) {
    return DateTimeApplet::getCurrentDate(dateFormat) + " " + DateTimeApplet::getCurrentTime(timeFormat);
}

QString DateTimeApplet::__getDisplayedData__(QString timeFormat) {
    return DateTimeApplet::getCurrentTime(timeFormat);
}
