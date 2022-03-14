QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

LIBS += -lX11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    applet.cpp \
    applets/appmenu/appmenu.cpp \
    applets/datetime/datetime.cpp \
    applets/favoriteApps/favoriteapps.cpp \
    applets/volume/volume.cpp \
    main.cpp \
    panel.cpp

HEADERS += \
    applet.h \
    applets/appmenu/appmenu.h \
    applets/datetime/datetime.h \
    applets/favoriteApps/favoriteapps.h \
    applets/volume/volume.h \
    panel.h

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    devnotes.txt \
    styles/dark.qss \
    styles/light.qss
