#ifndef DOOR_H
#define DOOR_H

#include "world.h"

class Door: public Tile
{
public:
    Door(int x, int y, float value, int targetLevel);
    int getTargetLevel();

private:
    int targetLevel;
};

#endif // DOOR_H
