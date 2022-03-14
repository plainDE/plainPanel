#include "../../applet.h"
#include "../../panel.h"

#ifndef FAVORITEAPPS_H
#define FAVORITEAPPS_H

struct FavoriteAppsUI {
    QWidget* listWidgetUI;
    QWidget* editFavoriteList;
};


class FavoriteApps
{
public:
    FavoriteAppsUI __createUI__(PanelLocation location, short panelHeight, QFont font, short buttonX);
    void updateList();
    void addFavoriteApp();
    void removeFavoriteApp();
};

#endif // FAVORITEAPPS_H
