#ifndef DATETIME_H
#define DATETIME_H

#include "../../applet.h"

#include <QCalendarWidget>

class DateTimeApplet : public Applet {
public:
    DateTimeApplet(ConfigManager* cfgMan,
                   Panel* parentPanel,
                   QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void internalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel);
    void repeatingAction(ConfigManager* cfgMan, Panel* parentPanel, bool showDate);
    void activate(ConfigManager* cfgMan, Panel* parentPanel);
    ~DateTimeApplet();

    QPushButton* mExternalWidget;


private:
    QString getTime();
    QString getDate();
    QString getDisplayedData(bool showDate);
    QString getDisplayedData();

    QCalendarWidget* mCalendarWidget;
    QString mTimeFormat;
    QString mDateFormat;
    bool mShowDate;

    PanelLayout mLayout;

    int mInterval;
    QTimer* mTimer;
};

#endif // DATETIME_H
