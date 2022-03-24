#ifndef PANEL_H
#define PANEL_H

#include <QWidget>

/* including QListWidget and QListWidgetItem here due to
 *  bug caused by including them in appmenu.h */
#include <QListWidget>
#include <QListWidgetItem>
#include <QJsonObject>

/* including XKBLib here due to ug caused by including
 * them in kblayout.h */
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Unsorted


QT_BEGIN_NAMESPACE
namespace Ui { class panel; }
QT_END_NAMESPACE


enum PanelLocation {
    top,
    bottom
};

class panel : public QWidget
{
    Q_OBJECT


public:   
    panel(QWidget *parent = nullptr);
    void updateFavApps(QJsonArray* newFavorites);
    ~panel();

public slots:
    void toggleAppMenu();
    void toggleCalendar();
    void toggleUserMenu();
    void filterAppsList();
    void setVolume();
    void updateAppletsData();

};
#endif // PANEL_H
