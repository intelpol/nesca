#-------------------------------------------------
#
# Project created by QtCreator 2015-02-24T13:25:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

PKGCONFIG += openssl
CONFIG += c++11

TARGET = nesca
TEMPLATE = app
LIBS += -lssh
INCLUDEPATH += /opt/Qt5.3.2/5.3/gcc_64/include/QtWidgets/
SOURCES +=\
        main.cpp \
    ActivityDrawerTh_HorNet.cpp \
    base64.cpp \
    CheckKey_Th.cpp \
    CheckProxy_Th.cpp \
    connector.cpp \
    DrawerTh_GridQoSScanner.cpp \
    DrawerTh_HorNet.cpp \
    DrawerTh_ME2Scanner.cpp \
    DrawerTh_QoSScanner.cpp \
    DrawerTh_VoiceScanner.cpp \
    irc_nmblinker.cpp \
    IRCPinger_Th.cpp \
    msgcheckerthread.cpp \
    nesca_3.cpp \
    nesca_startModule.cpp \
    oIRC_Th.cpp \
    piestat.cpp \
    progressbardrawer.cpp \
    STh.cpp \
    vercheckerthread.cpp \
    finder.cpp


HEADERS  += ActivityDrawerTh_HorNet.h \
    base64.h \
    CheckKey_Th.h \
    CheckProxy_Th.h \
    DrawerTh_GridQoSScanner.h \
    DrawerTh_HorNet.h \
    DrawerTh_ME2Scanner.h \
    DrawerTh_QoSScanner.h \
    DrawerTh_VoiceScanner.h \
    externData.h \
    externFunctions.h \
    irc_nmblinker.h \
    IRCPinger_Th.h \
    mainResources.h \
    msgcheckerthread.h \
    nesca_3.h \
    oIRC_Th.h \
    piestat.h \
    progressbardrawer.h \
    resource.h \
    STh.h \
    vercheckerthread.h

FORMS    += nesca_3.ui

RESOURCES += \
    nesca_3.qrc

OTHER_FILES += \
    nesca_3.rc
