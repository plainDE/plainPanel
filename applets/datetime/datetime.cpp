#include "datetime.h"

void DateTimeApplet::externalWidgetSetup() {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(QFont(mCfgMan->mFontFamily, mCfgMan->mFontSize));
    mExternalWidget->setObjectName("dateTimeButton");
    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);

    mTimeFormat = mCfgMan->mTimeFormat;
    mShowDate = mCfgMan->mShowDate;
    mDateFormat = mCfgMan->mDateFormat;

    // Make connections
    connect(static_cast<QPushButton*>(mExternalWidget),
            &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            mCalendarWidget->setSelectedDate(QDate::currentDate());
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
        }
    });
}

void DateTimeApplet::internalWidgetSetup() {
    mInternalWidget = new QWidget();

    // Geometry
    int width = 340, height = 250;
    preliminaryInternalWidgetSetup(width, height, false);

    // Main UI
    QVBoxLayout* mainLayout = new QVBoxLayout(mInternalWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mCalendarWidget = new QCalendarWidget();
    mCalendarWidget->setGridVisible(true);
    mCalendarWidget->setFirstDayOfWeek(mCfgMan->mFirstDayOfWeek);
    if (mCfgMan->mShowWeekNumbers) {
        mCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::ISOWeekNumbers);
    }
    else {
        mCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    }
    mainLayout->addWidget(mCalendarWidget);
}

void DateTimeApplet::repeatingAction() {
    QString data = getDisplayedData();
    static_cast<QPushButton*>(mExternalWidget)->setText(data);
}

void DateTimeApplet::repeatingAction(bool) {
    QString data = getDisplayedData(false);
    static_cast<QPushButton*>(mExternalWidget)->setText(data);
}

void DateTimeApplet::activate() {
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    mTimer->setTimerType(Qt::PreciseTimer);
    if (mShowDate) {
        connect(mTimer, &QTimer::timeout, this, [this]() {
            repeatingAction();
        });
    }
    else {
        connect(mTimer, &QTimer::timeout, this, [this]() {
            repeatingAction(false);
        });
    }

    // https://github.com/lxqt/lxqt-panel/blob/master/plugin-worldclock/lxqtworldclock.cpp
    /* Panel might not start exactly at hh:mm:ss:0000, therefore
     * first update should be earlier than interval. After that we
     * use 1000 ms as an interval between updates. This approach
     * lets us get time more precisely and save resources. */

    unsigned short delay = 1000 - ((QTime::currentTime().msecsSinceStartOfDay()) % 1000);
    if (mShowDate) {
        QTimer::singleShot(delay, Qt::PreciseTimer, this, [this]() {
            repeatingAction();
        });
    }
    else {
        QTimer::singleShot(delay, Qt::PreciseTimer, this, [this]() {
            repeatingAction(false);
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

QString DateTimeApplet::getDisplayedData() {
    if (mParentPanel->mPanelLayout == Horizontal) {
        return getDate() + " " + getTime();
    }
    else {  // Vertical
        return getDate() + "\n" + getTime();
    }
}

QString DateTimeApplet::getDisplayedData(bool) {
    return getTime();
}

DateTimeApplet::DateTimeApplet(ConfigManager* cfgMan,
                               Panel* parentPanel) : DynamicApplet(
                                                         "org.plainDE.dateTime",
                                                         cfgMan,
                                                         parentPanel,
                                                         1000
                                                     ) {
    // mLayout = parentPanel->mPanelLayout;
    // qDebug() << mLayout;
}

DateTimeApplet::~DateTimeApplet() {
    delete mExternalWidget;
}
