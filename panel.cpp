#include "panel.h"
#include "ui_panel.h"

#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
//include <QDesktopWidget>
#include <QScreen>
#include <QTimer>
#include <QJsonArray>
#include <QMultiMap>
#include <QVariantList>
#include <QIcon>
#include <QDir>


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "applets/clock/clock.h"
#include "applets/kblayout/kblayout.h"
#include "applets/volume/volume.h"
//include "applets/windowlist/windowlist.h"

#include "mainmenu.h"


QJsonObject config;
QVariantList obj;


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
    QList<QScreen*> screensList = QGuiApplication::screens();

    // Find primary QScreen
    QScreen* primaryScreen;
    QString primaryScreenName = config["primaryScreen"].toString();
    qDebug() << config["primaryScreen"];
    qDebug() << primaryScreenName;

    int x = 0, y = 0;
    for (int i = 0; i < screensList.length(); ++i) {
        if ((*screensList[i]).name() == primaryScreenName) {
            primaryScreen = screensList[i];
            break;
        }
        x += (*screensList[i]).size().width();
    }

    // Size & location
    int panelWidth = primaryScreen->size().width();
    int panelHeight = config["panelHeight"].toInt();
    if (config["panelLocation"].toString() == "bottom") {
        y = primaryScreen->size().height() - panelHeight;
    }

    w->setFixedSize(panelWidth, panelHeight);
    w->move(x, y);

    // Flags
    //w->setWindowFlags(w->windowFlags() | Qt::X11BypassWindowManagerHint);
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_X11NetWmWindowTypeDock);

}


void setPanelStyle(Ui::panel* ui, panel* w) {
    // theme & icons

    QString mainMenuIconPath = "", userIconPath = "";

    mainMenuIconPath = "/usr/share/plainDE/icons/menu-black.png";
    if (config["theme"].toString() == "dark") {
        mainMenuIconPath = "/usr/share/plainDE/icons/menu.png";
    }
    userIconPath = "/usr/share/plainDE/icons/user.png";

    ui->mainMenuPushButton->setIcon(QIcon(mainMenuIconPath));
    ui->userPushButton->setIcon(QIcon(userIconPath));

    // style ...
}


void setTime(Ui::panel* ui, bool showSeconds) {
    // Clock Applet
    QString currentTime = clockApplet::getCurrentTime(showSeconds);
    ui->timeLabel->setText(currentTime);
}

void setKbLayout(Ui::panel* ui, bool showFlag) {
    QString kbLayout = kblayoutApplet::getCurrentKbLayout();
    QString isoCode = kblayoutApplet::convertToISOCode(kbLayout);
    if (!showFlag) {
        ui->kbLayoutPushButton->setText(isoCode);
    }
    else {       
        ui->kbLayoutPushButton->setIcon(QIcon("/usr/share/plainDE/icons/flags/" + isoCode));
    }
}

void setRepeatingActions(Ui::panel* ui, panel* w) {
    // here we bring to life all QTimers

    // Time
    QTimer* timeSetTimer = new QTimer(w);
    timeSetTimer->setInterval(1000);
    if (!config["showSeconds"].toBool()) {
        setTime(ui,false);
        //w->connect(timeSetTimer, SIGNAL(timeout()), w, SLOT(setTime(ui,false)));
        //w->connect(timeSetTimer, &QTimer::timeout, w, setTime(ui,false));
        w->connect(timeSetTimer, &QTimer::timeout, w, [ui]() { setTime(ui,false); });
    }
    else {
        setTime(ui,true);
        w->connect(timeSetTimer, &QTimer::timeout, w, [ui]() { setTime(ui,true); });
    }
    timeSetTimer->start();


    // Keyboard Layout
    kblayoutApplet::initApplet();
    ui->kbLayoutPushButton->setText("");

    bool showFlag = config["showFlag"].toBool();
    qDebug() << showFlag;
    qDebug() << config;

    QTimer* kbLayoutTimer = new QTimer(w);
    kbLayoutTimer->setInterval(500);

    if (!showFlag) {
        setKbLayout(ui, false);
        w->connect(kbLayoutTimer, &QTimer::timeout, w, [ui]() { setKbLayout(ui, false); });
    }
    else {
        setKbLayout(ui, true);
        w->connect(kbLayoutTimer, &QTimer::timeout, w, [ui]() { setKbLayout(ui, true); });
    }

    kbLayoutTimer->start();


    // Window list

}

void initMenu() {

}

void autostart() {
    //
}


panel::panel(QWidget *parent): QMainWindow(parent), ui(new Ui::panel)
{
    ui->setupUi(this);

    //setTime(ui, false);

    readConfig();
    setPanelGeometry(this);
    setPanelStyle(ui, this);
    setRepeatingActions(ui, this);


}

panel::~panel()
{
    delete ui;
}

void panel::on_volumeDial_valueChanged(int value)
{
    volumeApplet::setVolume(value);
    ui->volumeLabel->setText(QString::number(value) + "%");
    qDebug() << value;
}


void panel::on_kbLayoutPushButton_clicked()
{
    // pass
}

