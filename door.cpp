#include "door.h"

Door::Door(int x, int y, float value, int targetLevel):Tile(x, y, value), targetLevel(targetLevel) {}

int Door::getTargetLevel(){
    return targetLevel;
}
