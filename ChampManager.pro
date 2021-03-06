QT += core widgets sql gui

CONFIG += c++17 #console

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += src/
SOURCES += \
        src/main.cpp \
    src/parser.cpp \
    src/dbhelper.cpp \
    src/resultswindow.cpp \
    src/mainwindow.cpp \
    src/appdata.cpp \

HEADERS += \
        src/parser.h \
        src/parserConsts.h \
        src/dbhelper.h \
        src/resultswindow.h \
        src/mainwindow.h \
        src/appdata.h \

FORMS += \
        src/mainwindow.ui \

RESOURCES += rsc/cm.qrc \

# test folder define
DEFINES += TESTDATA_PATH=\\\"$${PWD}/tests/testsdata/\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


