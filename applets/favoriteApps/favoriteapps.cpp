#include "favoriteapps.h"
#include "../../applet.h"


FavoriteAppsUI FavoriteApps::__createUI__(PanelLocation location, short panelHeight, QFont font, short buttonX) {
    QWidget* listWidgetUI = new QWidget;

    // Window flags
    listWidgetUI->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                  Qt::X11BypassWindowManagerHint);


    // Geometry
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    short listWidth = 100;
    short listHeight = 300;
    short ax = 0, ay = 0;
    if (location == top) {
        ay = panelHeight + 5;
    }
    else {
        ay = primaryScreen->geometry().height() - panelHeight - listHeight - 5;
    }
    if (primaryScreen->geometry().width() - buttonX >= listWidth) {
        ax = buttonX;
    }
    else {
        ax = buttonX - listWidth;
    }
    listWidgetUI->setFixedSize(listWidth, listHeight);
    listWidgetUI->move(ax, ay);


    // UI
    QVBoxLayout* listWidgetLayout = new QVBoxLayout;
    listWidgetLayout->setContentsMargins(0, 0, 0, 0);
    listWidgetUI->setLayout(listWidgetLayout);

    QListWidget* favoriteAppsList = new QListWidget;
    QPushButton* editFavoriteList = new QPushButton("Edit favorites...");

    listWidgetUI->layout()->addWidget(favoriteAppsList);
    listWidgetUI->layout()->addWidget(editFavoriteList);


}
