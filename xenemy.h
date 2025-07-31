#ifndef XENEMYH_H
#define XENEMYH_H

#include "world.h"

// XEnemy will becone a autoplay protagonist after defeated
class XEnemy: public Enemy
{
public:
    XEnemy(int x, int y, float value);
};

#endif // XENEMYH_H
