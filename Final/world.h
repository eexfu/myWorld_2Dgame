#ifndef WORLD_H
#define WORLD_H

#include "world_global.h"
#include <vector>
#include <memory>
#include <QObject>
#include <QImage>
#include <exception>

class WORLDSHARED_EXPORT Tile
{
public:
    Tile(int xPosition, int yPosition, float tileWeight);
    virtual ~Tile() =default;
    [[nodiscard]] float getValue() const noexcept {return value;} ;
    void setValue(float newValue) noexcept {value = newValue;};
    [[nodiscard]] constexpr int getXPos() const noexcept {return xPos;};
    [[nodiscard]] constexpr int getYPos() const noexcept {return yPos;};
    virtual void setXPos(int newPos) noexcept {xPos = newPos;};
    virtual void setYPos(int newPos) noexcept {yPos = newPos;};
    [[nodiscard]] constexpr bool operator== (const Tile & other) const noexcept
        {return (getXPos() == other.getXPos()) && (getYPos() == other.getYPos());};
    [[nodiscard]] virtual std::string serialize() noexcept ;

protected:
    int xPos;
    int yPos;
    float value;
};

class WORLDSHARED_EXPORT Enemy : public QObject, public Tile
{
    Q_OBJECT
public:
    Enemy(int xPosition, int yPosition, float strength);
    ~Enemy() override = default;
    [[nodiscard]] bool getDefeated() const noexcept {return defeated;}
    void setDefeated(bool value) noexcept {defeated = value; if (defeated) emit dead();};
    [[nodiscard]] std::string serialize() noexcept override;

  signals:
    void dead();

private:
    bool defeated {false};
};

class WORLDSHARED_EXPORT PEnemy: public Enemy
{
    Q_OBJECT
public:
    PEnemy(int xPosition, int yPosition, float strength);
    ~PEnemy() override = default;
    [[nodiscard]] float getPoisonLevel() const noexcept {return poisonLevel;};
    void setPoisonLevel(float value) noexcept {poisonLevel = value;};
    [[nodiscard]] std::string serialize() noexcept override;

public slots:
    bool poison();

signals:
    void poisonLevelUpdated(int value);

private:
    float poisonLevel;
};

class WORLDSHARED_EXPORT Protagonist: public QObject, public Tile
{
    Q_OBJECT
public:
    Protagonist();
    void setXPos(int newPos) noexcept override {if (xPos != newPos){xPos = newPos; emit posChanged(xPos, yPos);}}
    void setYPos(int newPos) noexcept override {if (yPos != newPos){yPos = newPos; emit posChanged(xPos, yPos);}}
    void setPos(int newX, int newY) noexcept {if (xPos != newX || yPos != newY) {xPos = newX; yPos = newY; emit posChanged(xPos, yPos);}}
    [[nodiscard]] float getHealth() const noexcept {return health;};
    void setHealth(float value) noexcept {health = value; emit healthChanged(static_cast<int>(health));}

    [[nodiscard]] float getEnergy() const noexcept {return energy;}
    void setEnergy(float value) noexcept {energy = value; emit energyChanged(static_cast<int>(energy));}
    [[nodiscard]] std::string serialize() noexcept override;

signals:
    void posChanged(int x, int y);
    void healthChanged(int h);
    void energyChanged(int e);

private:
    float health {100.0f};
    float energy {100.0f};
};

class WORLDSHARED_EXPORT World
{
public:
    World() = default;
    World(const World & other) = delete;
    World operator=(const World & ohter) = delete;
    //createWorld may throw a std::logic_error exception
    //pRatio is the percentage PEnemies of the total number of enemies
    void createWorld(QString filename, unsigned int nrOfEnemies, unsigned int nrOfHealthpacks, float pRatio = 0.25f);
    [[nodiscard]] std::vector<std::unique_ptr<Tile>> getTiles();
    [[nodiscard]] std::vector<std::unique_ptr<Enemy>> getEnemies();
    [[nodiscard]] std::vector<std::unique_ptr<Tile>> getHealthPacks();
    [[nodiscard]] std::unique_ptr<Protagonist> getProtagonist() const noexcept {return std::make_unique<Protagonist>();};
    [[nodiscard]] int getRows() const noexcept {return rows;};
    [[nodiscard]] int getCols() const noexcept {return cols;};

private:
    int rows, cols;
    QImage world;
    std::vector<std::unique_ptr<Tile>> tiles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Tile>> healthPacks;
};


#endif // WORLD_H
