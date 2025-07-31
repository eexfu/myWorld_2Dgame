QT       += core gui

TARGET = world
TEMPLATE = lib
CONFIG += c++20
DEFINES += WORLD_LIBRARY

SOURCES += world.cpp

HEADERS += world.h\
        world_global.h\
        pathfinder_class.h


#place where library will be put on deployment, replace this by your own operating system and map

win32 {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

