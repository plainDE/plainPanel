#ifndef WORKSPACES_H
#define WORKSPACES_H

#include <QtGlobal>
#include <KWindowSystem>
#include <KX11Extras>


class Workspaces
{
public:
    void setCurrentSpace(qint8 newSpace);
};

#endif // WORKSPACES_H
