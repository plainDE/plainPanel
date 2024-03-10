#ifndef DATETIME_H
#define DATETIME_H

#include "../../dynamicapplet.h"

#include <QCalendarWidget>

class DateTimeApplet : public DynamicApplet {
public:
    DateTimeApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    void internalWidgetSetup() override;
    void activate() override;
    ~DateTimeApplet();

public slots:
    void repeatingAction() override;
    void repeatingAction(bool);

private:
    QString getTime();
    QString getDate();
    QString getDisplayedData();
    QString getDisplayedData(bool);

    QCalendarWidget* mCalendarWidget;
    QString mTimeFormat;
    QString mDateFormat;
    bool mShowDate;

    PanelLayout mLayout;
};

#endif // DATETIME_H
