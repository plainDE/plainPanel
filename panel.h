#ifndef PANEL_H
#define PANEL_H

#include <QWidget>

/* including QListWidget and QListWidgetItem here due to
 *  bug caused by including them in appmenu.h */
#include <QListWidget>
#include <QListWidgetItem>


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
    ~panel();

public slots:
    void toggleAppMenu();
    void toggleCalendar();
    void filterAppsList();
    void setSystemVolume();
    void updateAppletsData();

};
#endif // PANEL_H
