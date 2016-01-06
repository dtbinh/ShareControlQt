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
INCLUDEPATH += "$$(myCode)\MultiAgent\libconfig\_src"
INCLUDEPATH += "$$(myCode)\MultiAgent\libcortex\_src"
INCLUDEPATH += "$$(myCode)\MultiAgent\ShareControl201509\_src2"
INCLUDEPATH += "$$(myLibrary)\Motion Analysis\SDK2_1.10.0\include"

SOURCES += main.cpp\
    RobotItem.cpp \
    MainWindow.cpp \
    ExternalControl.cpp \
    Util/CoordinateSystem.cpp \
    Util/RobotInfoItem.cpp \
    Util/UserArrow.cpp \
    CoreData.cpp \
    ObstacleItem.cpp \
    ControlNew.cpp \
    ExpA.cpp

HEADERS  += \
    RobotItem.h \
    MainWindow.h \
    zmq.hpp \
    ExternalControl.h \
    Util/Auxilary.h \
    Util/CoordinateSystem.h \
    Util/RobotInfoItem.h \
    Util/UserArrow.h \
    CoreData.h \
    ObstacleItem.h \
    ControlNew.h \
    ExpA.h

FORMS    += mainwindow.ui \
    controlnew.ui

Debug{
    LIBS += -L"$$(myCode)\MultiAgent\libnetagent\Debug\v120"  -llibnetagent
    LIBS += -L"$$(myCode)\MultiAgent\libconfig\Debug"         -llibconfig
    LIBS += -L"$$(myLibrary)\ZeroMQ\4_1_3\Debug\v120\dynamic" -llibzmq
    LIBS += -L"$$(myCode)\MultiAgent\libcortex\Debug"         -llibcortex
    LIBS += -L"$$(myLibrary)\Motion Analysis\SDK2_1.10.0\lib32" -lCortex_SDK
}

Release{
    LIBS += -L"$$(myCode)\MultiAgent\libnetagent\Release\v120"  -llibnetagent
    LIBS += -L"$$(myCode)\MultiAgent\libconfig\Release"         -llibconfig
    LIBS += -L"$$(myLibrary)\ZeroMQ\4_1_3\Release\v120\dynamic" -llibzmq
    LIBS += -L"$$(myCode)\MultiAgent\libcortex\Release"         -llibcortex
    LIBS += -L"$$(myLibrary)\Motion Analysis\SDK2_1.10.0\lib32" -lCortex_SDK
}

QMAKE_CXXFLAGS_DEBUG   -= -Zc:strictStrings
QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
