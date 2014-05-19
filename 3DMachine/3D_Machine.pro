#-------------------------------------------------
#
# Project created by QtCreator 2014-04-25T10:09:53
#
#-------------------------------------------------
QT       += serialport
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 3D_Machine
TEMPLATE = app


SOURCES += main.cpp\
        control_3dmachine.cpp \
    aboutform.cpp \
    dataprocess.cpp \
    Modbus.cpp

HEADERS  += control_3dmachine.h \
    aboutform.h \
    dataprocess.h \
    Modbus.h \
    modbus_protl.h

FORMS    += control_3dmachine.ui \
    aboutform.ui
