#ifndef GRAPHICVIEW_H
#define GRAPHICVIEW_H

#include <QGraphicsView>
#include "view.h"
#include "levelingcontroller.h"
#include <Qmap>

class QGraphicsScene;
class QGraphicsPixmapItem ;

class GraphicView : public QGraphicsView, public View
{
    Q_OBJECT

public:
    explicit GraphicView(QWidget *parent = nullptr);

    void updateProtagonistPosition(int x, int y) override;
    QGraphicsScene* getScene() override;
    void render(LevelData &world)override;
    void createPoisonGasAnimation(int x, int y) override;
    void createExplosionAnimation(int x, int y) override;
    void handleHealthPackPickup(int xPos, int yPos)override;
    void visualizeAttack(Protagonist* protagonist)override;
    void visualizeHealthPackPickup(Protagonist* protagonist)override;
    void visualizeMovement(Protagonist* protagonist, int newX, int newY)override;
    void visualizePoisoned(Protagonist* protagonist)override;
    void visualizeDeath(Protagonist* protagonist)override;


    void hide() override;

    bool isRender([[maybe_unused]] int x, [[maybe_unused]] int y) override { return false; }

    void setHealthScene(QGraphicsScene *healthScene);
    void setEnergyScene(QGraphicsScene *energyScene);

private slots:
    void updateHealthBar(int health)override;
    void updateEnergyBar(int energy)override;
    void handleEnemyDefeat(Enemy* enemy);

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *protagonistItem;
    QGraphicsPixmapItem *PEnemyItem;
    QGraphicsPixmapItem *healthPackItem;
    const int STEP = 10; // TODO: need to be adjusted afterward
    const int SIZE = 10;

    // Health bar attributes
    QGraphicsScene *healthScene;         // Scene for the health bar
    QGraphicsRectItem *healthBar;        // Rect item for the health bar visualization
    const int maxEH = 100;               // Maximum health value for scaling the health bar

    // Energy bar attributes
    QGraphicsScene *energyScene;
    QGraphicsRectItem *energyBar;

    // For testing purposes
    int protagonistHealth = maxEH;
    int protagonistEnergy = maxEH;               // Maximum health value for scaling the health bar

    // Mapping of Enemy to its graphical representation
    QMap<Enemy*, QGraphicsPixmapItem*> enemyItems;

    // Values necessary for visualization of poisoning
    bool isPoisoned = false;
    QTimer* poisonTimer = nullptr;


};

#endif // GRAPHICVIEW_H
