#include "graphicview.h"
#include "xenemy.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QMovie>
#include <iostream>
#include <QGraphicsItemAnimation>
#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QTimeLine>
#include <QTimer>


GraphicView::GraphicView(QWidget *parent)
    : QGraphicsView(parent)
    , scene(new QGraphicsScene(this))
{

}

void GraphicView::updateProtagonistPosition(int x, int y) {
    protagonistItem->setPos(x * STEP, y * STEP);
    this->centerOn(protagonistItem);

    // QGraphicsEllipseItem* cross = scene->addEllipse(x * STEP, y * STEP, 10, 10, QPen(Qt::NoPen), QBrush(Qt::red));
    // cross->setZValue(0);

    // QTimer::singleShot(3000, this, [this, cross]() {
    //     scene->removeItem(cross);
    //     delete cross;
    // });

    // The path followed by the protagonist is visualized
    // QGraphicsEllipseItem* cross = scene->addEllipse(x * STEP, y * STEP, 10, 10);
    // cross->setZValue(0);
    // cross->setBrush(Qt::red);
}

QGraphicsScene* GraphicView::getScene() {
    return scene;
}

void GraphicView::render(LevelData &world){
    //QGraphicsScene* scene_del = scene;
    scene = new QGraphicsScene(this);
    // delete scene_del;
    show();

    // Track previously created tiles
    static QList<QGraphicsRectItem*> tileItems;

    // Mark all existing tile items as unused (temporarily hide them)
    for (auto& tileItem : tileItems) {
        tileItem->setVisible(false);
    }

    // Update or create tiles
    int tileIndex = 0;
    tileItems.clear();
    for (const auto &tile : world.tiles) {
        QGraphicsRectItem* tileItem = nullptr;

        // Reuse existing tile item if available
        if (tileIndex < tileItems.size()) {
            tileItem = tileItems[tileIndex];
            tileItem->setVisible(true); // Make it visible again
        } else {
            // Create a new tile item if we run out of reusable ones
            tileItem = new QGraphicsRectItem();
            scene->addItem(tileItem);
            tileItems.append(tileItem);
        }

        // Update tile properties
        tileItem->setRect(tile->getXPos() * STEP, tile->getYPos() * STEP, SIZE, SIZE);
        float value = tile->getValue();
        QColor color;
        if (value == std::numeric_limits<float>::infinity()) {
            color = Qt::black;
        } else {
            float clampedValue = std::clamp(value, 0.0f, 1.0f);
            int gray = static_cast<int>(255.0f * (1.0f - clampedValue));
            gray = std::clamp(gray, 0, 255);
            color = QColor(gray, gray, gray);
            qDebug() << "Value:" << value << "Clamped:" << clampedValue << "Gray:" << gray;
        }
        // QColor color = (value == std::numeric_limits<float>::infinity()) ? Qt::black : Qt::white;
        tileItem->setBrush(QBrush(color));
        tileItem->setPen(Qt::NoPen);

        ++tileIndex;
    }

    // Remove unused tiles
    while (tileIndex < tileItems.size()) {
        QGraphicsRectItem* unusedItem = tileItems.takeLast();
        scene->removeItem(unusedItem);
        delete unusedItem;
    }

    // render enemies
    for (const auto &enemy : world.enemies) {
        QPixmap enemyIcon;
        if (dynamic_cast<PEnemy*>(enemy.get())) {
            if(enemy->getDefeated())
            {
                enemyIcon.load(":/actor_images/tombstone.png");
            }
            else{
                enemyIcon.load(":/actor_images/PEnemy.png");
                std::cout << "PEnemy: ";
            }

        }
        else if(dynamic_cast<XEnemy*>(enemy.get())) {
            std::cout << "XEnemyFriend: ";
            if(enemy->getDefeated())
            {
                enemyIcon.load(":/actor_images/tombstone.png");
            }
            else{
                enemyIcon.load(":/actor_images/XEnemy_2.png");
                std::cout << "XEnemy: ";
            }
        }
        else {
            std::cout << "Enemy: ";
            if(enemy->getDefeated())
            {
                enemyIcon.load(":/actor_images/tombstone.png");
            }
            else{
                enemyIcon.load(":/actor_images/Enemy.png");
                std::cout << "Enemy: ";
            }
        }

        enemyIcon = enemyIcon.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QGraphicsPixmapItem *enemyItem = new QGraphicsPixmapItem(enemyIcon);
        enemyItem->setPos(enemy->getXPos() * STEP, enemy->getYPos() * STEP);
        scene->addItem(enemyItem);
        std::cout << enemy->getXPos() << " " << enemy->getYPos() << std::endl;

        // Store the mapping between the Enemy object and its graphical item
        enemyItems[enemy.get()] = enemyItem;
        // Connect the Enemy's dead signal to the slot in GraphicView
        connect(enemy.get(), &Enemy::dead, this, [this, enemy = enemy.get()]() {
            handleEnemyDefeat(enemy);
        });
    }

    // render healthpacks
    for (const auto &healthPack : world.healthPacks) {
        QPixmap healthPackPixmap(":/actor_images/medicine.png");
        healthPackPixmap = healthPackPixmap.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QGraphicsPixmapItem *healthPackItem = new QGraphicsPixmapItem(healthPackPixmap);
        healthPackItem->setZValue(1);
        healthPackItem->setPos(healthPack->getXPos() * STEP, healthPack->getYPos() * STEP);
        scene->addItem(healthPackItem);

        std::cout << "healthPack: ";
        std::cout << healthPack->getXPos() << " " << healthPack->getYPos() << std::endl;
    }

    // render door
    QPixmap DoorPixmap(":/actor_images/door.png");
    DoorPixmap = DoorPixmap.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem *healthPackItem = new QGraphicsPixmapItem(DoorPixmap);
    healthPackItem->setZValue(1);
    healthPackItem->setPos(world.door->getXPos() * STEP, world.door->getYPos() * STEP);
    scene->addItem(healthPackItem);
    std::cout << "Door: ";
    std::cout << world.door->getXPos() << " " << world.door->getYPos() << std::endl;

    // render protagonist
    if (protagonistItem) {
        scene->removeItem(protagonistItem);
        delete protagonistItem;
        protagonistItem = nullptr;
    }
    QPixmap protagonist(":/actor_images/protagonist.png");
    protagonist = protagonist.scaled(STEP,STEP,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    protagonistItem = scene->addPixmap(protagonist);
    protagonistItem->setZValue(1);
    this->centerOn(protagonistItem);
    protagonistItem->setPos(world.protagonist->getXPos() * STEP, world.protagonist->getYPos() * STEP);

    // Set up the initial health bar
    int barWidth = 50;
    int barHeight = 250;
    healthBar = new QGraphicsRectItem(0, 0, barWidth, barHeight);
    healthBar->setBrush(QBrush(Qt::green)); // Green for full health
    healthBar->setPen(Qt::NoPen); // No border
    healthScene->addItem(healthBar);
    updateHealthBar(maxEH);

    // Set up the initial energy bar
    energyBar = new QGraphicsRectItem(0, 0, barWidth, barHeight);
    energyBar->setBrush(QBrush(Qt::blue)); // Green for full energy
    energyBar->setPen(Qt::NoPen); // No border
    energyScene->addItem(energyBar);
    updateEnergyBar(maxEH);

}

void GraphicView::updateHealthBar(int health) {
    int barHeight = 250; // Full height of the bar
    int barWidth = 50;   // Full width of the bar
    float healthRatio = static_cast<float>(health) / maxEH; // Ratio of health remaining

    // Update the health bar height
    int currentHeight = static_cast<int>(barHeight * healthRatio);

    // Adjust the health bar size and position
    healthBar->setRect(0, barHeight - currentHeight, barWidth, currentHeight);

    // Update color based on health percentage
    if (healthRatio > 0.5) {
        healthBar->setBrush(QBrush(Qt::green)); // Green for high health
    } else if (healthRatio > 0.2) {
        healthBar->setBrush(QBrush(Qt::yellow)); // Yellow for medium health
    } else {
        healthBar->setBrush(QBrush(Qt::red)); // Red for low health
    }
}

void GraphicView::updateEnergyBar(int energy) {
    int barHeight = 250; // Full height of the bar
    int barWidth = 50;   // Full width of the bar
    float energyRatio = static_cast<float>(energy) / maxEH; // Ratio of energy remaining

    // Update the energy bar height
    int currentHeight = static_cast<int>(barHeight * energyRatio);

    // Adjust the energy bar size and position
    energyBar->setRect(0, barHeight - currentHeight, barWidth, currentHeight);

    // Update color based on energy percentage
    if (energyRatio > 0.5) {
        energyBar->setBrush(QBrush(Qt::blue)); // Blue for high energy
    } else if (energyRatio > 0.2) {
        energyBar->setBrush(QBrush(Qt::cyan)); // Cyan for medium energy
    } else {
        energyBar->setBrush(QBrush(Qt::red)); // Red for low energy
    }
}

void GraphicView::handleEnemyDefeat(Enemy* enemy) {
    auto it = enemyItems.find(enemy);
    if (it != enemyItems.end()) {
        QGraphicsPixmapItem* enemyItem = it.value();
        QPixmap tombstoneIcon(":/actor_images/tombstone.png");
        tombstoneIcon = tombstoneIcon.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // Update the enemy item's pixmap
        enemyItem->setPixmap(tombstoneIcon);
        std::cout << "Enemy defeated at position (" << enemy->getXPos() << ", " << enemy->getYPos() << ")" << std::endl;
    }
}

void GraphicView::handleHealthPackPickup(int xPos, int yPos) {
    for (auto item : scene->items()) {
        if (item->pos() == QPointF(xPos * STEP, yPos * STEP)) {
            scene->removeItem(item);
            delete item;
            std::cout << "Health pack removed at position (" << xPos << ", " << yPos << ")" << std::endl;
            break;
        }
    }
}

void GraphicView::setHealthScene(QGraphicsScene *healthScene){
    this->healthScene = healthScene;
}

void GraphicView::setEnergyScene(QGraphicsScene *energyScene){
    this->energyScene = energyScene;
}

void GraphicView::createPoisonGasAnimation(int x, int y){
    // load image
    QPixmap poisonGasImage(":/effects/poison_gas.png");
    if (poisonGasImage.isNull()) {
        qDebug() << "Failed to load poison gas image!";
        return;
    }

    // create item
    poisonGasImage = poisonGasImage.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* poisonGasItem = new QGraphicsPixmapItem(poisonGasImage);
    poisonGasItem->setPos(x * STEP, y * STEP);
    poisonGasItem->setZValue(5);
    scene->addItem(poisonGasItem);

    // timeline of animation
    QTimeLine* timeLine = new QTimeLine(2000, this);
    timeLine->setFrameRange(0, 100);

    // create scaling animation
    QGraphicsItemAnimation* animation = new QGraphicsItemAnimation();
    animation->setItem(poisonGasItem);
    animation->setTimeLine(timeLine);

    // effect of animation: scale it
    for (int i = 0; i <= 100; ++i) {
        double scaleFactor = 1.0 + 0.01 * i;
        animation->setScaleAt(i / 100.0, scaleFactor, scaleFactor);
    }

    // remove item after timeout
    connect(timeLine, &QTimeLine::finished, [this, poisonGasItem]() {
        scene->removeItem(poisonGasItem);
        delete poisonGasItem;
    });

    // start timeline
    timeLine->start();
}

void GraphicView::createExplosionAnimation(int x, int y) {
    // Load explosion image
    QPixmap explosionImage(":/effects/explosion.png");
    if (explosionImage.isNull()) {
        qDebug() << "Failed to load explosion image!";
        return;
    }

    // Create item for explosion
    explosionImage = explosionImage.scaled(SIZE, SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* explosionItem = new QGraphicsPixmapItem(explosionImage);

    // Set position and Z value
    explosionItem->setPos(x * STEP, y * STEP);
    explosionItem->setZValue(6); // Ensure visibility over other elements

    // Set the transform origin point to the center of the item
    explosionItem->setTransformOriginPoint(SIZE / 2.0, SIZE / 2.0);

    scene->addItem(explosionItem);

    // Timeline for animation
    QTimeLine* timeLine = new QTimeLine(1000, this); // Shorter duration for explosion
    timeLine->setFrameRange(0, 100);

    // Create scaling animation
    QGraphicsItemAnimation* animation = new QGraphicsItemAnimation();
    animation->setItem(explosionItem);
    animation->setTimeLine(timeLine);

    // Effect of animation: scale
    for (int i = 0; i <= 100; ++i) {
        double scaleFactor = 1.0 + 0.02 * i; // Faster scaling for explosion effect
        animation->setScaleAt(i / 100.0, scaleFactor, scaleFactor);
    }

    // Create a fade effect
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect();
    explosionItem->setGraphicsEffect(opacityEffect);

    QTimeLine* fadeTimeLine = new QTimeLine(1000, this); // Same duration as scaling animation
    fadeTimeLine->setFrameRange(0, 100);
    connect(fadeTimeLine, &QTimeLine::frameChanged, [opacityEffect](int frame) {
        opacityEffect->setOpacity(1.0 - frame / 100.0); // Gradually decrease opacity
    });

    // Remove item after timeout
    connect(timeLine, &QTimeLine::finished, [this, explosionItem, opacityEffect]() {
        scene->removeItem(explosionItem);
        delete opacityEffect; // Remove effect
        delete explosionItem;
    });

    // Start animations
    timeLine->start();
    fadeTimeLine->start();
}

void GraphicView::hide() {
   // scene->clear(); // Clear the scene to remove old items
    QGraphicsView::hide(); // Hide the graphics view
}

void GraphicView::visualizeAttack([[maybe_unused]] Protagonist* protagonist) {
    if (!protagonistItem) return;

    for (int i = 0; i < 10; ++i) {
        QGraphicsEllipseItem* particle = new QGraphicsEllipseItem(
            protagonistItem->x() + (rand() % SIZE), protagonistItem->y() + (rand() % SIZE), 5, 5);
        particle->setBrush(Qt::red);
        scene->addItem(particle);

        // Animate the particle moving outward
        QTimer::singleShot(200, this, [=]() {
            particle->setPos(particle->x() + (rand() % 20 - 10), particle->y() + (rand() % 20 - 10));
        });

        // Remove the particle after 400ms
        QTimer::singleShot(400, this, [=, this]() {
            scene->removeItem(particle);
            delete particle;
        });
    }
}

void GraphicView::visualizeHealthPackPickup([[maybe_unused]] Protagonist* protagonist) {
    if (!protagonistItem) return; // Ensure the protagonist item exists

    // Create a timeline for the animation
    QTimeLine* timeline = new QTimeLine(500, this); // Duration of 500ms
    timeline->setFrameRange(0, 100); // Frame range for smooth animation

    // Create the graphics item animation
    QGraphicsItemAnimation* animation = new QGraphicsItemAnimation;
    animation->setItem(protagonistItem); // Set the protagonist item to animate
    animation->setTimeLine(timeline);

    // Define the scaling effect during the animation
    animation->setScaleAt(0.0, 1.0, 1.0);   // Initial scale
    animation->setScaleAt(0.5, 1.3, 1.3);   // Slightly enlarge at mid-animation
    animation->setScaleAt(1.0, 1.0, 1.0);   // Return to original size at the end

    // Clean up resources after the animation finishes
    connect(timeline, &QTimeLine::finished, this, [=]() {
        delete animation;
        delete timeline;
    });

    // Start the animation
    timeline->start();
}

void GraphicView::visualizeMovement([[maybe_unused]] Protagonist* protagonist, int newX, int newY) {
    if (protagonistItem) {
        QPointF oldPos = protagonistItem->pos();

        QPointF newPos(newX * STEP, newY * STEP);

        // Draw a line from the old position to the new position
        QGraphicsLineItem* traceLine = scene->addLine(oldPos.x() + SIZE / 2, oldPos.y() + SIZE / 2,
                                                      newPos.x() + SIZE / 2, newPos.y() + SIZE / 2,
                                                      QPen(QColorConstants::Svg::orange, 2));
        traceLine->setZValue(0);

        protagonistItem->setPos(newPos);
    }
}


void GraphicView::visualizePoisoned([[maybe_unused]] Protagonist* protagonist) {
    if (!protagonistItem) return;

    // If the protagonist is already poisoned, just reset the timer
    if (isPoisoned) {
        poisonTimer->start();
        return;
    }

    QPixmap originalPixmap = protagonistItem->pixmap();

    QPixmap purplePixmap = originalPixmap;
    QPainter painter(&purplePixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter.fillRect(purplePixmap.rect(), QColor(128, 0, 128, 150));
    painter.end();

    protagonistItem->setPixmap(purplePixmap);

    // Start pulsating effect (scale up and down)
    QTimer* pulsatingTimer = new QTimer(this);
    pulsatingTimer->setInterval(500); // Pulsate every 500ms (half a second)
    connect(pulsatingTimer, &QTimer::timeout, [=, this]() {
        static bool scaledUp = false;
        if (scaledUp) {
            protagonistItem->setScale(1.0);
        } else {
            protagonistItem->setScale(1.2);
        }
        scaledUp = !scaledUp;
    });
    pulsatingTimer->start();

    isPoisoned = true;

    // Create a timer that will reset after 3.5 seconds
    poisonTimer = new QTimer(this);
    poisonTimer->setSingleShot(true); // Only run once
    poisonTimer->setInterval(3500);   // 3.5 seconds duration

    // When the timer expires, reset the effect
    connect(poisonTimer, &QTimer::timeout, this, [=, this]() {
        protagonistItem->setPixmap(originalPixmap);
        protagonistItem->setScale(1.0);
        pulsatingTimer->stop();
        delete pulsatingTimer;

        isPoisoned = false;
    });

    poisonTimer->start();
}

void GraphicView::visualizeDeath([[maybe_unused]] Protagonist* protagonist) {
    if (protagonistItem) {
        QPixmap deathPixmap(":/actor_images/death.png");
        QPixmap scaledDeathPixmap = deathPixmap.scaled(SIZE, SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        protagonistItem->setPixmap(scaledDeathPixmap);
    }

    qDebug() << "Protagonist is dead. Death image displayed and scaled.";
}

