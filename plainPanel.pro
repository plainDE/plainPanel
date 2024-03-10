QT       += core gui network KWindowSystem concurrent multimedia

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
    applets/battery/battery.cpp \
    applets/clioutput/clioutput.cpp \
    applets/datetime/datetime.cpp \
    applets/kblayout/kblayout.cpp \
    applets/launcher/launcher.cpp \
    applets/localipv4/localipv4.cpp \
    applets/mpris/mpris.cpp \
    applets/snitray/snitray.cpp \
    applets/snitray/statusnotifierwatcher.cpp \
    applets/spacer/spacer.cpp \
    applets/splitter/splitter.cpp \
    applets/usermenu/powerdialog.cpp \
    applets/usermenu/usermenu.cpp \
    applets/volume/volume.cpp \
    applets/windowlist/windowlist.cpp \
    applets/workspaces/workspaces.cpp \
    configman.cpp \
    dbusintegration.cpp \
    dynamicapplet.cpp \
    initializer.cpp \
    main.cpp \
    panel.cpp \
    staticapplet.cpp

HEADERS += \
    applet.h \
    applets/appmenu/appmenu.h \
    applets/battery/battery.h \
    applets/clioutput/clioutput.h \
    applets/datetime/datetime.h \
    applets/kblayout/kblayout.h \
    applets/launcher/launcher.h \
    applets/localipv4/localipv4.h \
    applets/mpris/mpris.h \
    applets/snitray/snitray.h \
    applets/snitray/statusnotifierwatcher.h \
    applets/spacer/spacer.h \
    applets/splitter/splitter.h \
    applets/usermenu/powerdialog.h \
    applets/usermenu/usermenu.h \
    applets/volume/volume.h \
    applets/windowlist/windowlist.h \
    applets/workspaces/workspaces.h \
    configman.h \
    dbusintegration.h \
    dynamicapplet.h \
    initializer.h \
    panel.h \
    staticapplet.h

#CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
