#include <QProcess>

#include "../../applet.h"
#include "../../panel.h"

#ifndef USERMENU_H
#define USERMENU_H

struct userMenuUI {
    QWidget* userMenuWidget;
    QPushButton* settingsEntry;
    QPushButton* aboutEntry;
    QPushButton* logOutEntry;
    QPushButton* powerOffEntry;
    QPushButton* rebootEntry;
};

class UserMenuApplet {
public:
    userMenuUI __createUI__(QWidget* parent,
                            PanelLocation location,
                            short panelHeight,
                            QFont font,
                            short buttonX,
                            short buttonXRight,
                            QString theme,
                            qreal opacity);
    void about();
    void settings();
};

#endif // USERMENU_H
