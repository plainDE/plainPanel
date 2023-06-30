#include "datetime.h"
#include "../../panel.h"


void DateTimeApplet::createUI(Qt::DayOfWeek firstDay,
                              PanelLocation panelLocation,
                              QFont font,
                              int panelThickness,
                              int screenWidth,
                              int screenHeight,
                              int buttonCoord1,
                              int buttonCoord2,
                              double opacity) {
    // Window flags
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Geometry
    int width = 340, height = 250;
    int ax = 0, ay = 0;
    switch (panelLocation) {
        case Top:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = panelThickness + 5;
        break;

        case Bottom:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = screenHeight - panelThickness - height - 5;
        break;

        case Left:
            ax = panelThickness + 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;

        case Right:
            ax = screenWidth - panelThickness - width - 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;
    }
    this->setGeometry(ax, ay, width, height);
    //this->setFixedSize(width, height);
    //this->move(ax, ay);

    // Set font
    this->setFont(font);

    // Set opacity
    this->setWindowOpacity(opacity);

    // Calendar-specific settings
    this->setGridVisible(true);
    this->setFirstDayOfWeek(firstDay);
}

QString DateTimeApplet::getCurrentTime(QString timeFormat) {
    return QTime::currentTime().toString(timeFormat);
}

QString DateTimeApplet::getCurrentDate(QString dateFormat) {
    return QDate::currentDate().toString(dateFormat);
}

QString DateTimeApplet::__getDisplayedData__(QString timeFormat, QString dateFormat, PanelLayout panelLayout) {
    if (panelLayout == Horizontal) {
        return DateTimeApplet::getCurrentDate(dateFormat) + " " + DateTimeApplet::getCurrentTime(timeFormat);
    }
    else {
        return DateTimeApplet::getCurrentDate(dateFormat) + "\n" + DateTimeApplet::getCurrentTime(timeFormat);
    }
}

QString DateTimeApplet::__getDisplayedData__(QString timeFormat) {
    return DateTimeApplet::getCurrentTime(timeFormat);
}

DateTimeApplet::DateTimeApplet(Qt::DayOfWeek firstDay,
                               PanelLocation panelLocation,
                               QFont font,
                               int panelThickness,
                               int screenWidth,
                               int screenHeight,
                               int buttonCoord1,
                               int buttonCoord2,
                               double opacity) {
    createUI(firstDay,
             panelLocation,
             font,
             panelThickness,
             screenWidth,
             screenHeight,
             buttonCoord1,
             buttonCoord2,
             opacity);
}

DateTimeApplet::~DateTimeApplet() {

}
