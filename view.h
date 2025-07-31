#ifndef VIEW_H
#define VIEW_H

#include "levelingcontroller.h"
#include <QString>
#include <QGraphicsScene>
#include <QObject>

class View
{
public:
    virtual ~View() = default;

    virtual bool isRender(int x, int y) = 0;

    virtual void render(LevelData &world)=0;

    virtual void hide() = 0;

    virtual QGraphicsScene* getScene() = 0;

    virtual void createPoisonGasAnimation(int x, int y) = 0;
    virtual void createExplosionAnimation(int x, int y) = 0;

    virtual void updateHealthBar(int health) = 0;
    virtual void updateEnergyBar(int energy) = 0;
    virtual void updateProtagonistPosition(int x, int y) = 0;
    virtual void handleHealthPackPickup(int xPos, int yPos) = 0;
    virtual void visualizeAttack(Protagonist* protagonist) = 0;
    virtual void visualizeHealthPackPickup(Protagonist* protagonist) = 0;
    virtual void visualizeMovement(Protagonist* protagonist, int newX, int newY) = 0;
    virtual void visualizePoisoned(Protagonist* protagonist) = 0;
    virtual void visualizeDeath(Protagonist* protagonist) = 0;
};

#endif // VIEW_H
