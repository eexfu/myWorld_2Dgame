#include "levelingcontroller.h"
#include "xenemy.h"
#include <QString>
#include <QDebug>

LevelingController::LevelingController() {
    switchToLevel(0);
}


void LevelingController::switchToLevel(int levelIndex) {
    if(levelIndex < maxCacheSize) {
        if (levelCache.find(levelIndex) == levelCache.end()) {
            std::shared_ptr<World> world = std::make_shared<World>();

            world->createWorld(QString(":/world_images/maze%1.png").arg(levelIndex + 1), 40, 40, 0.25f);
            // world->createWorld(QString(":/world_images/worldmap.png"), 40, 40, 0.25f);

            LevelData data;
            data.tiles = world->getTiles();
            data.enemies = world->getEnemies();
            data.healthPacks = world->getHealthPacks();
            data.protagonist = world->getProtagonist();
            data.row = world->getRows();
            data.col = world->getCols();

            // Find a vaild tile for the initial position of protagonist
            for (const auto& tile : data.tiles) {
                if (tile->getValue() != std::numeric_limits<float>::infinity()) {
                    data.protagonist->setXPos(tile->getXPos());
                    data.protagonist->setYPos(tile->getYPos());
                    break;
                }
            }

            // Select a random valid position for the door
            std::shared_ptr<Door> door = nullptr;
            while (!door) {
                int randomIndex = std::rand() % data.tiles.size();
                Tile* tile = data.tiles[randomIndex].get();

                // Ensure the tile is valid and not occupied by Protagonist, Enemy, or HealthPack
                if (tile->getValue() != std::numeric_limits<float>::infinity() &&
                    tile->getXPos() != data.protagonist->getXPos() &&
                    tile->getYPos() != data.protagonist->getYPos()) {
                    bool isOccupied = false;

                    for (const auto& enemy : data.enemies) {
                        if (enemy->getXPos() == tile->getXPos() &&
                            enemy->getYPos() == tile->getYPos()) {
                            isOccupied = true;
                            break;
                        }
                    }

                    for (const auto& healthPack : data.healthPacks) {
                        if (healthPack->getXPos() == tile->getXPos() &&
                            healthPack->getYPos() == tile->getYPos()) {
                            isOccupied = true;
                            break;
                        }
                    }

                    if (!isOccupied) {
                        int nextLevel = currentLevelIndex < maxCacheSize - 1 ? currentLevelIndex + 1 : 0;
                        door = std::make_shared<Door>(tile->getXPos(), tile->getYPos(), tile->getValue(), nextLevel);
                        data.door = door;
                    }
                }
            }

            // convert Enemy into PEnemy
            bool flag = false;
            int i = 0, size = data.enemies.size();
            while(!flag && i < size) {
                // skip PEnemy
                if (dynamic_cast<PEnemy*>(data.enemies[i].get())) {
                    i++;
                    continue;
                }

                // convert Enemy into XEnemy
                std::unique_ptr<Enemy> enemy = std::move(data.enemies[i]);

                auto xenemy = std::make_unique<XEnemy>(
                    enemy->getXPos(),
                    enemy->getYPos(),
                    enemy->getValue()
                    );

                data.enemies[i] = std::move(xenemy); // replace as XEnemy
                flag = true;
            }
            if(!flag) qDebug() << "cannot convert XEnemy!!!";

            levelCache[levelIndex] = std::move(data);
        }

        currentLevelIndex = levelIndex;
    }
    else{
        currentLevelIndex = 0;
    }
}

std::vector<std::unique_ptr<Tile>>& LevelingController::getCurrentTiles() {
    return levelCache.at(currentLevelIndex).tiles;
}

std::vector<std::unique_ptr<Enemy>>& LevelingController::getCurrentEnemies() {
    return levelCache.at(currentLevelIndex).enemies;
}

std::vector<std::unique_ptr<Tile>>& LevelingController::getCurrentHealthPacks() {
    return levelCache.at(currentLevelIndex).healthPacks;
}

std::unique_ptr<Protagonist>& LevelingController::getCurrentProtagonist() {
    return levelCache.at(currentLevelIndex).protagonist;
}

std::shared_ptr<Door> LevelingController::getCurrentDoor(){
    return levelCache.at(currentLevelIndex).door;
}

LevelData& LevelingController::getCurrentWorld() {
    return levelCache.at(currentLevelIndex);
}

int LevelingController::getCurrentLevel(){
    return currentLevelIndex;
}

bool LevelingController::isEnterDoor(){
    return getCurrentProtagonist()->getXPos() == getCurrentDoor()->getXPos() && getCurrentProtagonist()->getYPos() == getCurrentDoor()->getYPos();
}
