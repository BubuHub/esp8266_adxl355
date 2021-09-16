QT       += core gui serialport charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG -= debug_and_release
unix: {
        CONFIG -= release
        CONFIG += debug
        OBJECTS_DIR = ./.build/obj
        MOC_DIR = ./.build/moc
        UI_DIR = ./.build/ui
        RCC_DIR = ./.build/rcc
}
QT += network
QMAKE_CXXFLAGS += -std=gnu++0x
win32: DEFINES += WINDOWS
NCLUDEPATH += .

TARGET = qt-logger
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

VERSION = 1.0.0
NAME = qt-logger

DEFINES +=  APP_VERSION=\\\"$$VERSION\\\"\
            APP_NAME=\\\"$$NAME\\\"\

SOURCES += *.cpp

HEADERS += *.h

FORMS += *.ui

win32:RC_FILE = winicon.rc
RESOURCES += images/images.qrc
include( qss/qss.pri )
