QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += $$PWD/sdk/inc \
    $$PWD/sdk/include

LIBS += -L$$PWD/sdk/lib \
    -lavcodec \
    -lavdevice \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lpostproc \
    -lswscale \
    -lswresample \
    -lSDL2

SOURCES += \
    VideoSlider.cpp \
    VideoWidget.cpp \
    main.cpp \
    mainwindow.cpp \
    $$PWD/sdk/src/IPlayer.cpp \
    sdk/src/Player.cpp

HEADERS += \
    VideoSlider.h \
    VideoWidget.h \
    mainwindow.h \
    $$PWD/sdk/inc/IPlayer.h \
    sdk/inc/Debug.h \
    sdk/src/Player.h

RC_ICONS=player.ico


FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon/icon.qrc
