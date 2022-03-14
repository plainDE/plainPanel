#include "panel.h"

#include <stdio.h>

#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScreen>
#include <QThread>
#include <QTimer>
#include <QJsonArray>
#include <QJsonValue>
#include <QIcon>
#include <QDir>
#include <QSize>
#include <QMap>

#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QDial>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QSpacerItem>

#include <X11/Xlib.h>
//include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include "applet.h"
#include "applets/appmenu/appmenu.h"
#include "applets/datetime/datetime.h"


Display* display;
QJsonObject config;
//QMap<QString,QWidget*> widgets;
// All applets
QMap<QString,QPushButton*> appletsButtons;
PanelLocation location;

QFont panelFont;
// We will delete these variables if some of applets are not activated:
AppMenu* menu;
menuUI _menuUI;
QList<App>* appList = new QList<App>;
QList<QString>* execList = new QList<QString>;

QHBoxLayout* windowListLayout = new QHBoxLayout;

DateTimeApplet _dateTime;
QWidget* calendarWidget;


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
    //w->setWindowFlags(w->windowFlags() | Qt::X11BypassWindowManagerHint);
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    w->show();
}

void setRepeatingActions(panel* w) {
    // here we bring to life main QTimer

    // time
    // kbLayout
    // XNextEvent

    /*QTimer* windowListTimer = new QTimer(w);
    windowListTimer->setInterval(1000);
    w->connect(windowListTimer, &QTimer::timeout, w, [windowList]() { updateWindowList(windowList); });
    windowListTimer->start();*/


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

    // Theme & icons
    /*QString mainMenuIconPath = "", userIconPath = "";

    mainMenuIconPath = "/usr/share/plainDE/artwork/plainIcons/png/32x32/menu-black.png";
    if (config["theme"].toString() == "dark") {
        mainMenuIconPath = "/usr/share/plainDE/artwork/plainIcons/png/32x32/menu-white.png";
    }*/

    // Layout
    QHBoxLayout* uiLayout = new QHBoxLayout;
    uiLayout->setContentsMargins(1, 1, 1, 1);
    w->setLayout(uiLayout);

    // Applets
    QVariantList activatedAppletsList = config["applets"].toArray().toVariantList();
    /* We could use QVariantList::contains, but this approach will not observe
     * order of placing applets, using loop. */
    location = (config["panelLocation"].toString() == "top") ? top : bottom;
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "appmenu") {
            QPushButton* appMenuPushButton = new QPushButton;
            appMenuPushButton->setFlat(true);
            w->layout()->addWidget(appMenuPushButton);
            appletsButtons["appMenuPushButton"] = appMenuPushButton;

            _menuUI = menu->__createUI__(location, config["panelHeight"].toInt(), panelFont, appMenuPushButton->x());

            // temporary (?)
            appMenuPushButton->setIcon(QIcon("/usr/share/plainDE/artwork/plainIcons/png/menuIcon.png"));

            appMenuPushButton->setText(" " + config["menuText"].toString());

            w->connect(appMenuPushButton, &QPushButton::clicked, w, &panel::toggleAppMenu);
        }

        else if (applet == "windowlist") {
            w->layout()->addItem(windowListLayout);
            w->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
        }

        else if (applet == "datetime") {
            QPushButton* dateTimePushButton = new QPushButton;
            dateTimePushButton->setFlat(true);
            w->layout()->addWidget(dateTimePushButton);
            appletsButtons["dateTimePushButton"] = dateTimePushButton;

            calendarWidget = _dateTime.__createUI__(location,
                                                    config["panelHeight"].toInt(),
                                                    panelFont,
                                                    dateTimePushButton->x());

            if (config["showDate"].toBool()) {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                dateTimePushButton->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }

            w->connect(dateTimePushButton, &QPushButton::clicked, w, &panel::toggleCalendar);
        }
    }

    setRepeatingActions(w);


    // style ...

}


void testPoint(panel* w) {
    //popen("/usr/bin/chromium", "r");
    // QListWidget* menuAppsList, QList<App>* menu, QListWidgetItem** itemList

    qDebug() << _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                               config["dateFormat"].toString());
}


// Slots
void panel::toggleAppMenu() {
    if (!_menuUI.menuWidget->isVisible()) {
        _menuUI.searchBox->clear();
        menu->buildMenu(_menuUI.appListWidget, appList, execList);
        _menuUI.menuWidget->show();
    }
    else {
        _menuUI.menuWidget->hide();
    }
}

void panel::toggleCalendar() {
    if (!calendarWidget->isVisible()) {
        calendarWidget->show();
    }
    else {
        calendarWidget->hide();
    }
}

void panel::filterAppsList() {
    menu->searchApps(_menuUI.appListWidget, _menuUI.searchBox->text());
}

void panel::setSystemVolume() {

}

void panel::updateAppletsData() {
    QVariantList activatedAppletsList = config["applets"].toArray().toVariantList();
    foreach (QVariant applet, activatedAppletsList) {
        if (applet == "datetime") {
            if (config["showDate"].toBool()) {
                appletsButtons["dateTimePushButton"]->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString(),
                                                           config["dateFormat"].toString()));
            }
            else {
                appletsButtons["dateTimePushButton"]->setText(
                            _dateTime.__getDisplayedData__(config["timeFormat"].toString()));
            }
        }
    }
}


panel::panel(QWidget *parent): QWidget(parent) {
    readConfig();
    setPanelGeometry(this);
    setPanelUI(this);

    //testPoint(this);
}

panel::~panel() {

}
