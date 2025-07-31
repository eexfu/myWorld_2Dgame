QT       += core gui
QT += widgets
# For example, add at the very bottom of your .pro:
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-deprecated
QMAKE_CXXFLAGS += -Wno-sign-compare


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    door.cpp \
    gamecontroller.cpp \
    graphicview.cpp \
    health.cpp \
    levelingcontroller.cpp \
    main.cpp \
    mainwindow.cpp \
    overlaycontroller.cpp \
    secondscreen.cpp \
    textview.cpp \
    view.cpp \
    xenemy.cpp

HEADERS += \
    door.h \
    gamecontroller.h \
    graphicview.h \
    health.h \
    levelingcontroller.h \
    mainwindow.h \
    myNode.h \
    overlaycontroller.h \
    secondscreen.h \
    textview.h \
    view.h \
    xenemy.h

FORMS += \
    mainwindow.ui \
    secondscreen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../FInal/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/release/ -lworld
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../FInal/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug/ -lworld


#INCLUDEPATH += $$PWD/../Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug
#DEPENDPATH += $$PWD/../Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/release/ -lworld
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug/ -lworld
else:unix: LIBS += -L$$PWD/Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/ -lworld

INCLUDEPATH += $$PWD/Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug
DEPENDPATH += $$PWD/Final/build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/debug

RESOURCES += \
    images.qrc \
    images.qrc
