#include "panel.h"

#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScreen>
#include <QTimer>
#include <QJsonArray>
#include <QJsonValue>
#include <QIcon>
#include <QMap>

#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QDial>
#include <QHBoxLayout>
#include <QSpacerItem>

#include <QX11Info>

#include "applet.h"
#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"
#include "applets/kblayout/kblayout.h"
#include "applets/usermenu/usermenu.h"
#include "applets/volume/volume.h"


Display* display = QX11Info::display();
QJsonObject config;
QMap<QString,QWidget*> appletWidgets;
PanelLocation location;
QFont panelFont;

AppMenu* menu;
menuUI _menuUI;

QHBoxLayout* windowListLayout = new QHBoxLayout;

DateTimeApplet _dateTime;
dateTimeUI _dateTimeUI;

KbLayoutApplet _kbLayout;

UserMenuApplet _userMenu;
userMenuUI _userMenuUI;

void readConfig() {
    // set globally readable variable for reading settings

    QString homeDirectory = getenv("HOME");
    QFile file;
    QString data;

    file.setFileName(homeDirectory + "/.config/plainDE/config.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data = file.readAll();
    file.close();
    config = QJsonDocument::fromJson(data.toUtf8()).object();
}

void basicInit(panel* w) {
    if (!QX11Info::isPlatformX11()) {
        qDebug() << "plainPanel currently works only on X11. Quitting...";
        QApplication::quit();
    }

    w->setWindowTitle("plainPanel");
    QDir::setCurrent(getenv("HOME"));
}

void setPanelGeometry(panel* w) {
    // size, location, monitor settings, window flags

    // Get screens
    //QList<QScreen*> screensList = QGuiApplication::screens();
    // QGuiApplication::primaryScreen()->name();

    // Find primary screen
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    // Size & location
    int ax = 0, ay = 0;
    int panelWidth = primaryScreen->size().width();
    int panelHeight = config["panelHeight"].toInt();
    if (config["panelLocation"].toString() == "bottom") {
        ay = primaryScreen->size().height() - panelHeight;
    }
    w->setFixedSize(panelWidth, panelHeight);
    w->move(ax, ay);

    // Flags
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}

void setRepeatingActions(panel* w) {
    // here we bring to life main QTimer for updating applets data

    QTimer* updateAppletsDataTimer = new QTimer(w);
    updateAppletsDataTimer->setInterval(500);
    w->connect(updateAppletsDataTimer, &QTimer::timeout, w, &panel::updateAppletsData);
    updateAppletsDataTimer->start();
}


void setPanelUI(panel* w) {
    // Set font
    panelFont.setFamily(config["fontFamily"].toString());
    panelFont.setPointSize(config["fontSize"].toInt());
    w->setFont(panelFont);

    // Set style
    // general

    /*if (config["theme"] == "light") {
        QFile stylesheetReader(":/styles/styles/general-light.qss");
        stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream styleSheet(&stylesheetReader);
        w->setStyleSheet(styleSheet.readAll());
    }*/

    // panel
    QFile stylesheetReader(":/styles/styles/panel.qss");
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    w->setStyleSheet(styleSheet.readAll());


    // Icons
    QIcon::setThemeName(config["iconTheme"].toString());


    // Layout
    QHBoxLayout* uiLayout = new QHBoxLayout;
    uiLayout->setContentsMargins(1, 1, 1, 1);
    w->setLayout(uiLayout);


    // Applets: show applets
    QFontMetrics fm(panelFont);
    location = (config["panelLocation"].toString() == "top") ? top : bottom;

    /* We could use QVariantList::contains, but this approach will not observe
     * order of placing applets, using loop. */
    QVariantList activatedAppletsList = config["applets"].toArray().toVariantList();
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            w->layout()->addWidget(appMenuPushButton);
            appletWidgets["appMenuPushButton"] = appMenuPushButton;

            // temporary (?)
            //appMenuPushButton->setIcon(QIcon("/usr/share/plainDE/artwork/plainIcons/png/menuIcon.png"));
            appMenuPushButton->setIcon(QIcon(":/img/menuIcon.png"));

            appMenuPushButton->setText(" " + config["menuText"].toString());
        }

        else if (applet == "windowlist") {
            // not realized yet

            w->layout()->addItem(windowListLayout);
            w->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
        }

        else if (applet == "datetime") {
            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            w->layout()->addWidget(dateTimePushButton);

            appletWidgets["dateTimePushButton"] = dateTimePushButton;

            if (config["showDate"].toBool()) {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }
        }

        else if (applet == "kblayout") {
            QPushButton* layoutName = new QPushButton;
            appletWidgets["layoutName"] = layoutName;

            short buttonWidth = fm.horizontalAdvance("ZZ");
            layoutName->setMaximumWidth(buttonWidth);
            layoutName->setFlat(true);

            w->layout()->addWidget(layoutName);
        }

        else if (applet == "usermenu") {
            QPushButton* userMenuPushButton = new QPushButton;
            userMenuPushButton->setFlat(true);
            userMenuPushButton->setText(getenv("USER"));
            appletWidgets["userMenuPushButton"] = userMenuPushButton;
            userMenuPushButton->setObjectName("userMenuPushButton");

            short userMenuWidth = fm.horizontalAdvance(getenv("USER")) + 20;
            userMenuPushButton->setMaximumWidth(userMenuWidth);

            userMenuPushButton->setIcon(QIcon::fromTheme("computer"));

            w->layout()->addWidget(userMenuPushButton);
        }

        else if (applet == "volume") {
            QDial* volumeDial = new QDial;
            volumeDial->setMinimum(0);
            volumeDial->setMaximum(100);
            volumeDial->setValue(50);
            volumeDial->setMaximumWidth(25);

            QLabel* volumeLabel = new QLabel("50%");
            volumeLabel->setMaximumWidth(fm.horizontalAdvance("100%"));

            w->layout()->addWidget(volumeDial);
            w->layout()->addWidget(volumeLabel);

            VolumeApplet::setVolume(50);

            appletWidgets["volumeDial"] = volumeDial;
            appletWidgets["volumeLabel"] = volumeLabel;
        }
    }

    w->show();

    // Applets: set actions
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "appmenu") {
            _menuUI = menu->__createUI__(location, config["panelHeight"].toInt(),
                                        panelFont, appletWidgets["appMenuPushButton"]->x(),
                                        appletWidgets["appMenuPushButton"]->geometry().topRight().x(),
                                        config["appMenuTriangularTabs"].toBool());
            w->connect(static_cast<QPushButton*>(appletWidgets["appMenuPushButton"]), &QPushButton::clicked, w, &panel::toggleAppMenu);
        }

        else if (applet == "datetime") {
            Qt::DayOfWeek day;
            QString configDay = config["firstDayOfWeek"].toString();
            // temp
            if (configDay == "Monday") {
                day = Qt::Monday;
            }
            else if (configDay == "Tuesday") {
                day = Qt::Tuesday;
            }
            else if (configDay == "Wednesday") {
                day = Qt::Wednesday;
            }
            else if (configDay == "Thursday") {
                day = Qt::Thursday;
            }
            else if (configDay == "Friday") {
                day = Qt::Friday;
            }
            else if (configDay == "Saturday") {
                day = Qt::Saturday;
            }
            else {
                day = Qt::Sunday;
            }

            _dateTimeUI = _dateTime.__createUI__(location,
                                                    config["panelHeight"].toInt(),
                                                    panelFont,
                                                    appletWidgets["dateTimePushButton"]->pos().x(),
                                                    appletWidgets["dateTimePushButton"]->geometry().topRight().x(),
                                                    day);

            w->connect(static_cast<QPushButton*>(appletWidgets["dateTimePushButton"]), &QPushButton::clicked, w, &panel::toggleCalendar);
        }

        else if (applet == "kblayout") {
            _kbLayout.__init__();
            static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
        }

        else if (applet == "usermenu") {
            _userMenuUI = _userMenu.__createUI__(location,
                                                 config["panelHeight"].toInt(),
                                                 panelFont,
                                                 appletWidgets["userMenuPushButton"]->pos().x(),
                                                 appletWidgets["userMenuPushButton"]->geometry().topRight().x());

            w->connect(static_cast<QPushButton*>(appletWidgets["userMenuPushButton"]), &QPushButton::clicked, w, &panel::toggleUserMenu);
        }

        else if (applet == "volume") {
            w->connect(static_cast<QDial*>(appletWidgets["volumeDial"]), &QDial::valueChanged, w, &panel::setVolume);
        }
    }


    setRepeatingActions(w);


    // style ...

}


// Slots
void panel::toggleAppMenu() {    
    if (!_menuUI.menuWidget->isVisible()) {
        _menuUI.searchBox->clear();
        menu->buildMenu(_menuUI.appListWidget, "");
        menu->buildFavMenu(_menuUI.favListWidget, config["favApps"].toArray().toVariantList());
        _menuUI.tabWidget->setCurrentIndex(0);
        _menuUI.menuWidget->show();
    }
    else {
        _menuUI.menuWidget->hide();
    }
}

void panel::toggleCalendar() {
    if (!_dateTimeUI.calendarWidget->isVisible()) {
        _dateTimeUI.calendarWidget->show();
    }
    else _dateTimeUI.calendarWidget->hide();
}

void panel::toggleUserMenu() {
    if (!_userMenuUI.userMenuWidget->isVisible()) {
        _userMenuUI.userMenuWidget->show();
    }
    else _userMenuUI.userMenuWidget->hide();
}

void panel::filterAppsList() {
    menu->buildMenu(_menuUI.appListWidget, _menuUI.searchBox->text());
}

void panel::setVolume() {
    short newValue = static_cast<QDial*>(appletWidgets["volumeDial"])->value();
    VolumeApplet::setVolume(newValue);
    static_cast<QLabel*>(appletWidgets["volumeLabel"])->setText(QString::number(newValue) + "%");
}

void panel::updateAppletsData() {
    QVariantList activatedAppletsList = config["applets"].toArray().toVariantList();
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "datetime") {
            if (config["showDate"].toBool()) {
               static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                static_cast<QPushButton*>(appletWidgets["dateTimePushButton"])->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }
        }

        else if (applet == "kblayout") {
            static_cast<QPushButton*>(appletWidgets["layoutName"])->setText(_kbLayout.getCurrentKbLayout());
        }
    }
}




void testPoint(panel* w) {
    // here you can put your code to test

    //popen("/usr/bin/chromium", "r");
    // QListWidget* menuAppsList, QList<App>* menu, QListWidgetItem** itemList

    /*qDebug() << _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                               config["dateFormat"].toString());*/


}

panel::panel(QWidget *parent): QWidget(parent) {
    basicInit(this);
    readConfig();
    setPanelGeometry(this);
    setPanelUI(this);

    testPoint(this);
}

panel::~panel() {

}
