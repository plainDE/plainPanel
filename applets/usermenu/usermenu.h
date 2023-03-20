#ifndef USERMENU_H
#define USERMENU_H

#include <QWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QFile>

#include "../../panel.h"

class UserMenu : public QWidget {
    Q_OBJECT

public:
    void createUI(Panel* parentPanel,
                  QObject* execHolder,
                  PanelLocation panelLocation,
                  int panelThickness,
                  int screenWidth,
                  int screenHeight,
                  QFont font,
                  int buttonCoord1,
                  int buttonCoord2,
                  QString stylesheet,
                  double opacity);
    UserMenu(Panel* parentPanel,
             QObject* execHolder,
             PanelLocation panelLocation,
             int panelThickness,
             int screenWidth,
             int screenHeight,
             QFont font,
             int buttonCoord1,
             int buttonCoord2,
             QString stylesheet,
             double opacity);
    ~UserMenu();

signals:
    void panelShouldQuit();
};

#endif // USERMENU_H
