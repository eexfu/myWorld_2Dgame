#ifndef LEVELINGCONTROLLER_H
#define LEVELINGCONTROLLER_H

#include <vector>
#include <memory>
#include "world.h"
#include "door.h"

struct LevelData {
    std::vector<std::unique_ptr<Tile>> tiles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Tile>> healthPacks;
    std::unique_ptr<Protagonist> protagonist;
    std::shared_ptr<Door> door;
    int row;
    int col;
};

class LevelingController {
public:
    LevelingController();
    void switchToLevel(int levelIndex);
    std::vector<std::unique_ptr<Tile>>& getCurrentTiles();
    std::vector<std::unique_ptr<Enemy>>& getCurrentEnemies();
    std::vector<std::unique_ptr<Tile>>& getCurrentHealthPacks();
    std::unique_ptr<Protagonist>& getCurrentProtagonist();
    std::shared_ptr<Door> getCurrentDoor();
    int getCurrentLevel();
    LevelData& getCurrentWorld();
    bool isEnterDoor();

private:
    std::unordered_map<int, LevelData> levelCache;
    int currentLevelIndex = -1;
    const int maxCacheSize = 3;
};

#endif // LEVELINGCONTROLLER_H
