QT       += core gui KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets dbus

CONFIG += c++11
CONFIG += x11

LIBS += -lX11
LIBS += -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    applet.cpp \
    applets/appmenu/appmenu.cpp \
    applets/datetime/datetime.cpp \
    applets/kblayout/kblayout.cpp \
    applets/sni-tray/tray.cpp \
    applets/usermenu/usermenu.cpp \
    applets/volume/volume.cpp \
    applets/windowlist/windowlist.cpp \
    applets/workspaces/workspaces.cpp \
    main.cpp \
    panel.cpp

HEADERS += \
    applet.h \
    applets/appmenu/appmenu.h \
    applets/datetime/datetime.h \
    applets/kblayout/kblayout.h \
    applets/sni-tray/tray.h \
    applets/usermenu/usermenu.h \
    applets/volume/volume.h \
    applets/windowlist/windowlist.h \
    applets/workspaces/workspaces.h \
    panel.h

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    devnotes.txt \
    styles/general-light.qss \
    styles/gnome-colors-light.qss \
    styles/gradient-dark.qss \
    styles/gradient-light.qss \
    styles/still-dark.qss \
    styles/still-light.qss
