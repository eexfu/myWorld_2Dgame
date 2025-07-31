#ifndef MYNODE_H
#define MYNODE_H

#include "world.h"

class MyNode: public Tile {
public:
    float f, g, h;
    bool visited = false;
    bool closed = false;
    MyNode* prev = nullptr;

    MyNode(int x, int y, float value)
        : Tile(x, y, value), f(0), g(0), h(0), prev(nullptr) {}
};

#endif // MYNODE_H
