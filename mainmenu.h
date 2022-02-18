#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>

namespace Ui {
class mainMenu;
}

class mainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit mainMenu(QWidget *parent = nullptr);
    ~mainMenu();

private:
    Ui::mainMenu *ui;
};

#endif // MAINMENU_H
