#ifndef PANEL_H
#define PANEL_H

#include <QMainWindow>
#include <QEvent>

//include <X11/Xlib.h>

QT_BEGIN_NAMESPACE
namespace Ui { class panel; }
QT_END_NAMESPACE

class panel : public QMainWindow
{
    Q_OBJECT

    public:
        panel(QWidget *parent = nullptr);
        ~panel();

    private slots:
        void on_volumeDial_valueChanged(int value);
        void on_kbLayoutPushButton_clicked();

    private:
        Ui::panel *ui;

};


#endif // PANEL_H
