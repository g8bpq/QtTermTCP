
QT += core gui
QT += network
QT += widgets
QT += serialport
QT += multimedia


TARGET = QtTermTCP
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        QtTermTCP.cpp\
        TermTCPCommon.cpp\ 
        TabDialog.cpp  \
        AGWCode.cpp \
        ax25.c \
        UZ7HOUtils.c \
        ax25_l2.c\
        utf8Routines.cpp

HEADERS  += QtTermTCP.h\
        TabDialog.h

FORMS    += QtTermTCP.ui\
        ListenPort.ui \
        AGWParams.ui \
        VARAConfig.ui \
        KISSConfig.ui \
		ColourConfig.ui \
		YAPPRxSize.ui \
        AGWConnect.ui \
		AlertSetup.ui

RESOURCES += QtTermTCP.qrc

RC_ICONS = QtTermTCP.ico

ICON = QtTermTCP.icns

QMAKE_LFLAGS += -no-pie
