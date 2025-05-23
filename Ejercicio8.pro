QT       += core gui network
QT += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    appwidget.cpp \
    dragdroparea.cpp \
    historywindow.cpp \
    main.cpp \
    login.cpp

HEADERS += \
    appwidget.h \
    dragdroparea.h \
    historywindow.h \
    login.h

FORMS += \
    dragdroparea.ui \
    login.ui

SOURCES += \
    registro.cpp
HEADERS += \
    registro.h
FORMS += \
    registro.ui