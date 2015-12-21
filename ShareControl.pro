#-------------------------------------------------
#
# Project created by QtCreator 2015-09-11T20:58:38
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = MultiRobots
TEMPLATE = app

INCLUDEPATH += "$$(myLibrary)\ZeroMQ\4_1_3\include"
INCLUDEPATH += "$$(myCode)\MultiAgent\libnetagent\_src"
INCLUDEPATH += "$$(myCode)\MultiAgent\ShareControl201509\_src2"

SOURCES += main.cpp\
    ControlPanel.cpp \
    RobotItem.cpp \
    MainWindow.cpp \
    ExternalControl.cpp \
    Util/CoordinateSystem.cpp \
    Util/RobotInfoItem.cpp \
    Util/UserArrow.cpp \
    CoreData.cpp

HEADERS  += \
    RobotItem.h \
    MainWindow.h \
    ControlPanel.h \
    zmq.hpp \
    ExternalControl.h \
    Util/Auxilary.h \
    Util/CoordinateSystem.h \
    Util/RobotInfoItem.h \
    Util/UserArrow.h \
    CoreData.h

FORMS    += mainwindow.ui \
            controlpanel.ui

Debug{
    LIBS += -L"$$(myCode)\MultiAgent\libnetagent\Debug\v120" -llibnetagent
    LIBS += -L"$$(myLibrary)\ZeroMQ\4_1_3\Debug\v120\dynamic" -llibzmq
}

Release{
    LIBS += -L"$$(myCode)\MultiAgent\libnetagent\Release\v120" -llibnetagent
    LIBS += -L"$$(myLibrary)\ZeroMQ\4_1_3\Release\v120\dynamic" -llibzmq
}
