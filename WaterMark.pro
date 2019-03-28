#-------------------------------------------------
#
# Project created by QtCreator 2019-03-20T16:48:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WaterMark
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        sources/main.cpp \
        sources/mainwindow.cpp \
        sources/editor.cpp \
        sources/preview.cpp \
        sources/cropform.cpp \
        sources/watermarkform.cpp \
        sources/preset.cpp \
        sources/watermark.cpp \
        sources/aboutdialog.cpp \
        sources/licensedialog.cpp \
        sources/reportissuedialog.cpp

HEADERS += \
        sources/mainwindow.hpp \
        sources/editor.hpp \
        sources/preview.hpp \
        sources/cropform.hpp \
        sources/watermarkform.hpp \
        sources/preset.hpp \
        sources/watermark.hpp \
        sources/aboutdialog.hpp \
        sources/licensedialog.hpp \
        sources/reportissuedialog.hpp

FORMS += \
        ui/mainwindow.ui \
        ui/cropform.ui \
        ui/watermarkform.ui \
        ui/aboutdialog.ui \
        ui/licensedialog.ui \
        ui/reportissuedialog.ui

TRANSLATIONS += \
        languages/fr_fr.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
        ressources/icons.qrc

DISTFILES += \
        README.md \
        LICENSE.md
