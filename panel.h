#ifndef PANEL_H
#define PANEL_H

#include <QWidget>

/* including QListWidget and QListWidgetItem here due to
 *  bug caused by including them in appmenu.h */
#include <QListWidget>
#include <QListWidgetItem>
#include <QJsonObject>

/* including X libs here due to bug caused by including
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
#undef Bool


QT_BEGIN_NAMESPACE
namespace Ui { class panel; }
QT_END_NAMESPACE


enum PanelLocation {
    top,
    bottom
};

class panel : public QWidget {
    Q_OBJECT


public:   
    panel(QWidget *parent = nullptr);
    void updateGeometry();
    void updateFavApps(QJsonArray* newFavorites);
    void animation(panel* w); 
    ~panel();


public slots:
    void toggleAppMenu();
    void toggleCalendar();
    void toggleUserMenu();
    void filterAppsList();
    void setVolume();
    //void updateWindowList();
    //void updateWorkspaces();
    //void updateAppletsData();


    // updating applets data
    void updateDateTime(bool);  // Date & Time applet (Time + Date)
    void updateDateTime();      // Date & Time applet (Time only)
    void updateKbLayout();      // Keyboard layout applet (ISO code)
    void updateKbLayout(bool);  // Keyboard layout applet (flag)
    void updateWinList();       // Window List applet
    void updateWorkspaces();    // Workspaces applet
    void updateLocalIPv4();       // Local IP applet

    void freeUnusedMemory(bool quit);
};
#endif // PANEL_H
