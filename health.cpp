#include "health.h"
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QMovie>

Health::Health(QGraphicsItem * parent): QGraphicsView(), health(100), maxHealth(100)
{
    // Create the health bar
    healthBar = new QGraphicsRectItem(0, 0, 100, 10, parent);
    healthBar->setBrush(QBrush(Qt::green));
    healthBar->setPen(QPen(Qt::black, 2));
    scene()->addItem(healthBar);
}

void Health::setHealth(int value)
{
    health = qMax(0, qMin(value, maxHealth));

    // Update the size of the health bar proportionally
    qreal healthRatio = static_cast<qreal>(health) / maxHealth;
    healthBar->setRect(0, 0, 100 * healthRatio, 10);

    // Change color based on health percentage
    if (healthRatio > 0.5) {
        healthBar->setBrush(QBrush(Qt::green)); // Healthy
    } else if (healthRatio > 0.2) {
        healthBar->setBrush(QBrush(Qt::yellow)); // Warning
    } else {
        healthBar->setBrush(QBrush(Qt::red)); // Critical
    }
}

void Health::decreaseHealth(int amount)
{
    setHealth(health - amount); // Reduce health and update bar
}

int Health::getHealth() const
{
    return health;
}
