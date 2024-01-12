#include "datetime.h"


void DateTimeApplet::externalWidgetSetup(ConfigManager* cfgMan,
                                         Panel* parentPanel) {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(QFont(cfgMan->mFontFamily, cfgMan->mFontSize));
    mExternalWidget->setObjectName("dateTimeButton");
    mExternalWidget->setFlat(true);

    mTimeFormat = cfgMan->mTimeFormat;
    mShowDate = cfgMan->mShowDate;
    mDateFormat = cfgMan->mDateFormat;

    // Make connections
    connect(mExternalWidget, &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            mCalendarWidget->setSelectedDate(QDate::currentDate());
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
        }
    });
}

void DateTimeApplet::internalWidgetSetup(ConfigManager* cfgMan,
                                         Panel* parentPanel) {
    mInternalWidget = new QWidget();

    // Geometry
    int width = 340, height = 250;
    preliminaryInternalWidgetSetup(mInternalWidget,
                                   mExternalWidget,
                                   cfgMan,
                                   parentPanel,
                                   width,
                                   height,
                                   false);

    // Main UI
    QVBoxLayout* mainLayout = new QVBoxLayout(mInternalWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mCalendarWidget = new QCalendarWidget();
    mCalendarWidget->setGridVisible(true);
    mCalendarWidget->setFirstDayOfWeek(cfgMan->mFirstDayOfWeek);
    if (cfgMan->mShowWeekNumbers) {
        mCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::ISOWeekNumbers);
    }
    else {
        mCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    }
    mainLayout->addWidget(mCalendarWidget);
}

void DateTimeApplet::repeatingAction(ConfigManager* cfgMan, Panel* parentPanel) {
    QString data = getDisplayedData();
    mExternalWidget->setText(data);
}

void DateTimeApplet::repeatingAction(ConfigManager* cfgMan, Panel* parentPanel, bool showDate) {
    QString data = getDisplayedData(true);
    mExternalWidget->setText(data);
}

void DateTimeApplet::activate(ConfigManager* cfgMan, Panel* parentPanel) {
    mInterval = 1000;
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    mTimer->setTimerType(Qt::PreciseTimer);

    if (cfgMan->mShowDate) {
        connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel, true);
        });
    }
    else {
        connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel);
        });
    }

    // https://github.com/lxqt/lxqt-panel/blob/master/plugin-worldclock/lxqtworldclock.cpp
    /* Panel might not start exactly at hh:mm:ss:0000, therefore
     * first update should be earlier than interval. After that we
     * use 1000 ms as an interval between updates. This approach
     * lets us get time more precisely and save resources. */

    unsigned short delay = 1000 - ((QTime::currentTime().msecsSinceStartOfDay()) % 1000);
    if (cfgMan->mShowDate) {
        QTimer::singleShot(delay, Qt::PreciseTimer, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel, true);
        });
    }
    else {
        QTimer::singleShot(delay, Qt::PreciseTimer, this, [this, cfgMan, parentPanel]() {
            repeatingAction(cfgMan, parentPanel);
        });
    }

    QTimer::singleShot(delay, Qt::PreciseTimer, mTimer, SLOT(start()));
}

QString DateTimeApplet::getTime() {
    return QTime::currentTime().toString(mTimeFormat);
}

QString DateTimeApplet::getDate() {
    return QDate::currentDate().toString(mDateFormat);
}

QString DateTimeApplet::getDisplayedData(bool) {
    if (mLayout == Horizontal) {
        return getDate() + " " + getTime();
    }
    else {  // Vertical
        return getDate() + "\n" + getTime();
    }
}

QString DateTimeApplet::getDisplayedData() {
    return getTime();
}

DateTimeApplet::DateTimeApplet(ConfigManager* cfgMan,
                               Panel* parentPanel,
                               QString additionalInfo) : Applet(cfgMan, parentPanel, additionalInfo) {
    mLayout = parentPanel->mPanelLayout;
}

DateTimeApplet::~DateTimeApplet() {
    delete mExternalWidget;
}
