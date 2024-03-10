#ifndef WORKSPACES_H
#define WORKSPACES_H

#include "../../staticapplet.h"

#include <KWindowSystem>

class WorkspacesApplet : public StaticApplet {
    Q_OBJECT

public:
    WorkspacesApplet(ConfigManager* cfgMan, Panel* parentPanel);
    void externalWidgetSetup() override;
    ~WorkspacesApplet();

private:
    void addButtons();
    void setWorkspace(int n);
    QString getButtonText(int n);
    void accentCurrentDesktop();

    QString mAccent;
    int mCountWorkspaces;
    QList<QPushButton*> mButtonList;
    QString mInactiveStylesheet;
    QString mActiveStylesheet;
};

#endif // WORKSPACES_H
