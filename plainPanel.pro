QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
LIBS += -lX11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    applets/clock/clock.cpp \
    applets/kblayout/kblayout.cpp \
    applets/volume/volume.cpp \
    main.cpp \
    mainMenu/mainmenu.cpp \
    mainmenu.cpp \
    panel.cpp

HEADERS += \
    applets/clock/clock.h \
    applets/kblayout/kblayout.h \
    applets/volume/volume.h \
    mainMenu/mainmenu.h \
    mainmenu.h \
    panel.h

FORMS += \
    mainmenu.ui \
    panel.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    assets/flags/de.png \
    assets/flags/es.png \
    assets/flags/fr.png \
    assets/flags/it.png \
    assets/flags/nl.png \
    assets/flags/ru.png \
    assets/flags/ua.png \
    assets/flags/us.png \
    config.json \
    dependencies.txt
