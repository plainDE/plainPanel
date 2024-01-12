#ifndef WORKSPACES_H
#define WORKSPACES_H

#include "../../applet.h"

#include <KWindowSystem>

class WorkspacesApplet : public Applet {
    Q_OBJECT

public:
    WorkspacesApplet(ConfigManager* cfgMan, Panel* parentPanel, QString additionalInfo);
    void externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel);
    ~WorkspacesApplet();

    QWidget* mExternalWidget;

private:
    void addButtons(ConfigManager* cfgMan, Panel* parentPanel);
    void setWorkspace(int n);
    QString getButtonText(ConfigManager* cfgMan, int n);
    void accentCurrentDesktop(ConfigManager* cfgMan,
                              Panel* parentPanel);

    QString mAccent;
    int mCountWorkspaces;
    QList<QPushButton*> mButtonList;
    QString mInactiveStylesheet;
    QString mActiveStylesheet;
};

#endif // WORKSPACES_H
