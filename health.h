#ifndef HEALTH_H
#define HEALTH_H
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include "view.h"

class Health : public QGraphicsView, public View
{
    Q_OBJECT // Required for signals and slots

public:
    explicit Health(QGraphicsItem* parent = nullptr);
    void setHealth(int value);
    void decreaseHealth(int amount);
    int getHealth() const;

private:
    int health;
    QGraphicsRectItem* healthBar;
    int maxHealth;
};

#endif // HEALTH_H
