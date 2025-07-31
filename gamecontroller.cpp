#include "gamecontroller.h"
#include <QDebug>
#include <memory>
#include <QTimer>
#include <QMessageBox>
using namespace std;

bool GameController::isGameOver = false;
bool GameController::isAutoPlay = false;

GameController::GameController(std::shared_ptr<OverlayController> overlayController, std::shared_ptr<LevelingController> levelingController,  QObject *parent)
    : QObject(parent)
    , overlayController(overlayController)
{
    this->overlayController = overlayController;
    this->levelingController = levelingController;
    initializeCommands();  // Populate the command map during initialization
}

void GameController::GAMEOVER(){
    if(!isGameOver){
        QMessageBox::critical(nullptr, "Game Over", "YOU DIED!");
        GameController::isGameOver = true;
        isGameOver = true;
    }
}

void GameController::handleKeyInput(QKeyEvent *event) {

    int viewIndex=overlayController->getCurrentViewIndex();

    if(viewIndex==1)
    {
        switch (event->key()) {
        //case Qt::Key_W: dy = -1; break;
        //case Qt::Key_S: dy = 1;  break;
        //case Qt::Key_A: dx = -1; break;
        //case Qt::Key_D: dx = 1;  break;
        case Qt::Key_G: overlayController->switchView();overlayController->renderCurrentView(levelingController->getCurrentWorld());break;
        // case Qt::Key_T: overlayController->switchView();overlayController->renderCurrentView(levelingController->getCurrentWorld());break;
        case Qt::Key_P: {
            isAutoPlay = !isAutoPlay;
            if(isAutoPlay) autoplay();
            else clearQueue();
            break;
        }
        default: return;
        }

    }
    else
    {



    int dx = 0, dy = 0;

    switch (event->key()) {
        case Qt::Key_W: dy = -1; break;
        case Qt::Key_S: dy = 1;  break;
        case Qt::Key_A: dx = -1; break;
        case Qt::Key_D: dx = 1;  break;
        case Qt::Key_G: overlayController->switchView();overlayController->renderCurrentView(levelingController->getCurrentWorld());break;
        // case Qt::Key_T: overlayController->switchView();overlayController->renderCurrentView(levelingController->getCurrentWorld());break;
        case Qt::Key_P: {
            isAutoPlay = !isAutoPlay;
            if(isAutoPlay) autoplay();
            else clearQueue();
            break;
        }
        default: return;
    }

    overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), dx, dy);

    }
}

void GameController::handleSwitchScreen()
{
//     overlayController->switchView();overlayController->renderCurrentView(levelingController->getCurrentWorld());
//     overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), 0, 0);

 }


void GameController::clearQueue(){
    std::queue<std::pair<int, int>> empty1;
    std::queue<int> empty2;
    std::swap(moveQueue, empty1);
    std::swap(enemyQueue, empty2);
    std::swap(healthPackPathQueue, empty1);
}

std::vector<int> GameController::planPath(int x, int y) {
    LevelData &world = levelingController->getCurrentWorld();
    const unsigned int width = world.col;
    int s = world.protagonist->getYPos() * width + world.protagonist->getXPos();
    int d = y * width + x;

    std::cout << "from: " << world.protagonist->getXPos() << " " << world.protagonist->getYPos() << std::endl;
    std::cout << "goto: " << x << " " << y << std::endl;

    if (s < 0 || s >= (int)world.tiles.size() || d < 0 || d >= (int)world.tiles.size()) {
        std::cerr << "Out of range! s=" << s << ", d=" << d << std::endl;
        return {}; // Return empty path to prevent invalid memory access
    }

    Tile start(world.tiles[s]->getXPos(), world.tiles[s]->getYPos(), world.tiles[s]->getValue());
    Tile destination(world.tiles[d]->getXPos(), world.tiles[d]->getYPos(), world.tiles[d]->getValue());

    std::vector<MyNode> myNodes;
    for(size_t i = 0; i < world.tiles.size(); i++){
        myNodes.emplace_back(world.tiles[i]->getXPos(), world.tiles[i]->getYPos(), world.tiles[i]->getValue());
    }

    Comparator<MyNode> comp = [](const MyNode& a, const MyNode& b) {
        return a.f > b.f;
    };

    helper_func<Tile> costf = []([[maybe_unused]]const Tile& from, const Tile& to) {
        return to.getValue();
    };

    helper_func<Tile> distf = [](const Tile& a, const Tile& b) {
        return std::abs(a.getXPos() - b.getXPos()) + std::abs(a.getYPos() - b.getYPos());
    };

    float heuristicWeight = 1.0f;

    PathFinder<MyNode, Tile> pathfinder = PathFinder<MyNode, Tile>(myNodes, &start, &destination, comp, width, costf, distf, heuristicWeight);

    return pathfinder.A_star();
}

int GameController::findNearestHealthPack(LevelData &world) {
    float minDist = std::numeric_limits<float>::max();
    int index = 0;
    for (size_t i = 0; i < world.healthPacks.size(); i++) {
        float dist = std::abs(world.protagonist->getXPos() - world.healthPacks[i]->getXPos()) +
                     std::abs(world.protagonist->getYPos() - world.healthPacks[i]->getYPos());
        if (dist < minDist) {
            minDist = dist;
            index = i;
        }
    }
    return index;
}

void GameController::processNextMove() {
    if(!isGameOver && isAutoPlay){
        const int speed = 100;
        LevelData &world = levelingController->getCurrentWorld();

        // If currently heading to a health pack and the healthPackPathQueue is not empty, just proceed along the saved path
        if (headingToHealthPack && !healthPackPathQueue.empty()) {
            cout << "hp is up or get hp: BEFORE---------------" << endl;
            cout << "protagonist'hp: " << world.protagonist->getHealth() << endl;
            cout << "enemy's value: " << world.enemies[enemyQueue.front()]->getValue() << endl;
            cout << "HEALTH_THRESHOLD: " << HEALTH_THRESHOLD << endl;
            auto move = healthPackPathQueue.front();
            healthPackPathQueue.pop();
            overlayController->updateProtagonistPosition(world, move.first, move.second);

            // After this step, if health is still below the threshold and we still have path, continue
            // If this path finishes (healthPackPathQueue empty) but we haven't reached the health pack yet, maybe we need to find the path again
            // But we assume this path can definitely reach the health pack.

            // Check if we reached the health pack
            // If reached the health pack (or health recovered), cancel headingToHealthPack
            if (world.protagonist->getHealth() >= world.enemies[enemyQueue.front()]->getValue()
                || world.protagonist->getHealth() >= HEALTH_THRESHOLD
                || healthPackPathQueue.empty()) {
                headingToHealthPack = false;
                cout << "hp is up or get hp: AFTER---------------" << endl;
                cout << "protagonist'hp: " << world.protagonist->getHealth() << endl;
                cout << "enemy's value: " << world.enemies[enemyQueue.front()]->getValue() << endl;
                cout << "HEALTH_THRESHOLD: " << HEALTH_THRESHOLD << endl;
            }

            QTimer::singleShot(speed, this, &GameController::processNextMove);
            return;
        }

        // If not heading to a health pack, take instructions from the normal moveQueue
        if (!moveQueue.empty()) {
            auto move = moveQueue.front();
            moveQueue.pop();
            overlayController->updateProtagonistPosition(world, move.first, move.second);

            if ((world.protagonist->getHealth() < world.enemies[enemyQueue.front()]->getValue()
                || world.protagonist->getHealth() < HEALTH_THRESHOLD)
                && !headingToHealthPack
                && !world.healthPacks.empty()) {
                // First time health is below threshold, start planning path to health pack
                std::cout << "hp is low, try to find a hp" << std::endl;
                cout << "protagonist'hp: " << world.protagonist->getHealth() << endl;
                cout << "enemy's value: " << world.enemies[enemyQueue.front()]->getValue() << endl;
                cout << "HEALTH_THRESHOLD: " << HEALTH_THRESHOLD << endl;
                int hpIndex = findNearestHealthPack(world);
                const std::unique_ptr<Tile>& hp = world.healthPacks[hpIndex];
                std::cout << "hp's position: " << hp->getXPos() << " " << hp->getYPos() << std::endl;
                if (hp) {
                    std::vector<int> hpPath = planPath(hp->getXPos(), hp->getYPos());
                    if (!hpPath.empty()) {
                        // Clear current move queue
                        std::queue<std::pair<int, int>> empty;
                        std::swap(moveQueue, empty);

                        // Convert health pack path into move instructions and store in healthPackPathQueue
                        while (!healthPackPathQueue.empty()) {
                            healthPackPathQueue.pop();
                        }

                        for (int step : hpPath) {
                            int dx = 0, dy = 0;
                            switch (step) {
                            case 0: dx = 0; dy = -1; break;
                            case 1: dx = 1; dy = -1; break;
                            case 2: dx = 1; dy = 0; break;
                            case 3: dx = 1; dy = 1; break;
                            case 4: dx = 0; dy = 1; break;
                            case 5: dx = -1; dy = 1; break;
                            case 6: dx = -1; dy = 0; break;
                            case 7: dx = -1; dy = -1; break;
                            default:
                                std::cerr << "Error: invalid move from pathfinder!" << std::endl;
                                break;
                            }
                            healthPackPathQueue.push({dx, dy});
                        }

                        // Set headingToHealthPack status
                        headingToHealthPack = true;

                        // Do not immediately recursively call processNextMove, use QTimer to schedule
                        QTimer::singleShot(speed, this, &GameController::processNextMove);
                        return;
                    }
                }
            }

            // Normal case: check for enemies and continue next move
            if (!enemyQueue.empty()) {
                const std::unique_ptr<Enemy>& currentEnemy = world.enemies[enemyQueue.front()];
                if (currentEnemy && world.protagonist->getXPos() == currentEnemy->getXPos() &&
                    world.protagonist->getYPos() == currentEnemy->getYPos()) {
                    std::cout << "Enemy defeated!" << std::endl;
                    enemyQueue.pop();
                }
            }

            QTimer::singleShot(speed, this, &GameController::processNextMove);
        } else {
            // If no more moves, process next enemy
            executeMoves();
        }
    }
}

void GameController::executeMoves() {
    if (moveQueue.empty()) {
        // If no moves, proceed with next enemy
        if (!enemyQueue.empty()) {
            std::cout << "try to find an enemy" << std::endl;
            int enemyIndex = enemyQueue.front();
            enemyQueue.pop();
            LevelData &world = levelingController->getCurrentWorld();
            const std::unique_ptr<Enemy>& currentEnemy = world.enemies[enemyIndex];
            std::cout << "currentEnemyPos: " << currentEnemy->getXPos() << " " << currentEnemy->getYPos() << std::endl;
            // Plan path to current enemy
            std::vector<int> path = planPath(currentEnemy->getXPos(), currentEnemy->getYPos());
            if (path.empty()) {
                std::cout << "No path found to enemy!" << std::endl;
                executeMoves();
                return;
            }

            // Add path to move queue
            for (int move : path) {
                int dx = 0, dy = 0;
                switch (move) {
                case 0: dx = 0; dy = -1; break;
                case 1: dx = 1; dy = -1; break;
                case 2: dx = 1; dy = 0; break;
                case 3: dx = 1; dy = 1; break;
                case 4: dx = 0; dy = 1; break;
                case 5: dx = -1; dy = 1; break;
                case 6: dx = -1; dy = 0; break;
                case 7: dx = -1; dy = -1; break;
                default:
                    std::cerr << "Error: invalid move from pathfinder! move: " << move << std::endl;
                    break;
                }
                moveQueue.push({dx, dy});
            }

            // Start executing moves
            processNextMove();
        } else {
            std::cout << "All enemies have been defeated!" << std::endl;
            GAMEOVER();
        }
    }
}


void GameController::autoplay() {
    LevelData &world = levelingController->getCurrentWorld();

    // If no enemies, return directly
    if (world.enemies.empty()) {
        std::cout << "No enemies to attack!" << std::endl;
        return;
    }

    // Sort enemies by distance to protagonist
    std::sort(world.enemies.begin(), world.enemies.end(), [&](std::unique_ptr<Enemy> &a, std::unique_ptr<Enemy> &b) -> bool {
        // int dx_a = world.protagonist->getXPos() - a->getXPos();
        // int dy_a = world.protagonist->getYPos() - a->getYPos();
        // int dx_b = world.protagonist->getXPos() - b->getXPos();
        // int dy_b = world.protagonist->getYPos() - b->getYPos();
        // return (std::abs(dx_a) + std::abs(dy_a)) < (std::abs(dx_b) + std::abs(dy_b));
        return a->getValue() < b->getValue();
    });

    // Create a queue to store enemies to attack
    while (!enemyQueue.empty()) {
        enemyQueue.pop();
    }
    for (size_t i = 0; i < world.enemies.size(); i++) {
        enemyQueue.push(i);
    }

    // Initialize execution
    executeMoves();
}


void GameController::handleTextCommand(const QString& command) {
    if (commandMap.contains(command)) {

        commandMap[command]();  // Execute the associated action


    }else if (command.startsWith("gotoxy")) {
        // Example format: "gotoXY 5,7"

        // Split on space to separate "gotoXY" from "5,7"
        QStringList parts = command.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            qDebug() << "Usage: gotoXY x,y (e.g. gotoXY 5,7)";
            return;
        }

        // Now parse the "5,7" part
        QString coords = parts[1];  // e.g. "5,7"
        QStringList xy = coords.split(',');
        if (xy.size() < 2) {
            qDebug() << "Usage: gotoXY x,y (e.g. gotoXY 5,7)";
            return;
        }

        // Convert to integers
        bool ok1 = false, ok2 = false;
        int x = xy[0].toInt(&ok1);
        int y = xy[1].toInt(&ok2);

        if (!ok1 || !ok2) {
            qDebug() << "Invalid coordinates. Usage: gotoXY x,y (e.g. gotoXY 5,7)";

            return;
        }
        goToIndexX=x;goToIndexY=y;

        commandMap["gotoxy"]();


    }
    else {
        qDebug() << "Invalid command:" << command;
    }

}

void GameController::initializeCommands() {
    // Movement commands
    commandMap["up"] = [this]() {
        overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), 0, -1);
    };
    commandMap["down"] = [this]() {
        overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), 0, 1);
    };
    commandMap["left"] = [this]() {
        overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), -1, 0);
    };
    commandMap["right"] = [this]() {
        overlayController->updateProtagonistPosition(levelingController->getCurrentWorld(), 1, 0);
    };

    // Navigate to the nearest health pack
    commandMap["health"] = [this]() {
        navigateToNearestHealthPack();
    };

    commandMap["enemy"] = [this]() {
        navigateToNearestEnemy();
    };

    commandMap["gotoxy"] = [this]() {
        gotoXy();
    };



}
void GameController::gotoXy()
{
    if(goToIndexX< 0||goToIndexY<0)
    {
        qDebug() << "No path to X and Y";
        return;
    }
    LevelData &world = levelingController->getCurrentWorld();

    int protagonistX = world.protagonist->getXPos();
    int protagonistY = world.protagonist->getYPos();
    Tile start(protagonistX, protagonistY,
               world.tiles[protagonistX + protagonistY * world.col]->getValue());
    Tile destination(goToIndexX,
                     goToIndexY,
                     world.tiles[goToIndexX + goToIndexY * world.col]->getValue());


    std::vector<int> path = findPathToDestination(world, start, destination);
    if (path.empty()) {
        qDebug() << "No path to X and Y";
        return;
    }

    executePath(world, path);

    goToIndexX=-1;goToIndexY=-1;
}


std::vector<int> GameController::findPathToDestination(LevelData &world, const Tile &start, const Tile &destination) {
    const unsigned int width = world.col;

    std::vector<MyNode> myNodes;
    for (size_t i = 0; i < world.tiles.size(); i++) {
        myNodes.emplace_back(world.tiles[i]->getXPos(), world.tiles[i]->getYPos(), world.tiles[i]->getValue());
    }

    Comparator<MyNode> comp = [](const MyNode &a, const MyNode &b) {
        return a.f > b.f;
    };

    // Cost function with energy check
    helper_func<Tile> costf = [&world](const Tile &from, const Tile &to) {
        float energyCost = to.getValue();  // Use tile value as the energy cost
        if (world.protagonist->getEnergy() < energyCost) {
            return std::numeric_limits<float>::infinity();  // Block path if insufficient energy
        }
        return energyCost;
    };

    helper_func<Tile> distf = [](const Tile &a, const Tile &b) {
        return std::abs(a.getXPos() - b.getXPos()) + std::abs(a.getYPos() - b.getYPos());
    };

    float heuristicWeight = 1.0f;
    PathFinder<MyNode, Tile> pathfinder(myNodes, &start, &destination, comp, width, costf, distf, heuristicWeight);

    return pathfinder.A_star();
}


void GameController::navigateToNearestEnemy() {
    LevelData &world = levelingController->getCurrentWorld();

    int protagonistX = world.protagonist->getXPos();
    int protagonistY = world.protagonist->getYPos();
    Tile start(protagonistX, protagonistY,
               world.tiles[protagonistX + protagonistY * world.col]->getValue());

    // 1) Find the nearest enemy
    Enemy* nearestEnemy = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (const auto &enemy : world.enemies) {
        if(enemy->getDefeated())
            continue;
        int enemyX = enemy->getXPos();
        int enemyY = enemy->getYPos();

        float distance = std::abs(protagonistX - enemyX) + std::abs(protagonistY - enemyY);
        if (distance < minDistance) {
            minDistance = distance;
            nearestEnemy = enemy.get();
        }
    }

    // 2) Check if any enemy was found
    if (!nearestEnemy) {
        qDebug() << "No enemies available.";
        return;
    }

    // 3) Build the destination tile
    Tile destination(nearestEnemy->getXPos(),
                     nearestEnemy->getYPos(),
                     nearestEnemy->getValue());

    // 4) Find path from start to destination
    std::vector<int> path = findPathToDestination(world, start, destination);
    if (path.empty()) {
        qDebug() << "No path to the nearest enemy found.";
        return;
    }

    // 5) Execute the path
    executePath(world, path);
}




void GameController::executePath(LevelData &world, const std::vector<int> &path) {
    const unsigned int width = world.col;
    int currentX = world.protagonist->getXPos();
    int currentY = world.protagonist->getYPos();

    float totalEnergyCost = 0.0f;  // Track the total energy used

    for (int move : path) {
        int dx = 0, dy = 0;
        switch (move) {
        case 0: dx = 0; dy = -1; break;
        case 1: dx = 1; dy = -1; break;
        case 2: dx = 1; dy = 0; break;
        case 3: dx = 1; dy = 1; break;
        case 4: dx = 0; dy = 1; break;
        case 5: dx = -1; dy = 1; break;
        case 6: dx = -1; dy = 0; break;
        case 7: dx = -1; dy = -1; break;
        }

        int newX = currentX + dx;
        int newY = currentY + dy;

        // Calculate energy cost for this move
        float tileCost = world.tiles[newX + newY * width]->getValue();
        totalEnergyCost += tileCost;

        // Check if energy is sufficient before making the move
        if (world.protagonist->getEnergy() < totalEnergyCost) {
            qDebug() << "Path found, but the protagonist does not have enough energy. Aborting path.";
            return;
        }

        // Update position and execute the move
        overlayController->updateProtagonistPosition(world, dx, dy);
        currentX = newX;
        currentY = newY;
    }

    qDebug() << "Path executed successfully. Total energy used:" << totalEnergyCost;
}

void GameController::navigateToNearestHealthPack() {
    LevelData &world = levelingController->getCurrentWorld();

    int protagonistX = world.protagonist->getXPos();
    int protagonistY = world.protagonist->getYPos();
    Tile start(protagonistX, protagonistY, world.tiles[protagonistX + protagonistY * world.col]->getValue());

    // Find the nearest health pack
    Tile *nearestHealthPack = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (const auto &healthPack : world.healthPacks) {
        if(healthPack->getValue()==-1)
        continue;
        int healthPackX = healthPack->getXPos();
        int healthPackY = healthPack->getYPos();

        float distance = std::abs(protagonistX - healthPackX) + std::abs(protagonistY - healthPackY);
        if (distance < minDistance) {
            minDistance = distance;
            nearestHealthPack = healthPack.get();
        }
    }

    if (!nearestHealthPack) {
        qDebug() << "No health packs available.";
        return;
    }

    Tile destination(nearestHealthPack->getXPos(), nearestHealthPack->getYPos(), nearestHealthPack->getValue());

    // Find path to health pack
    std::vector<int> path = findPathToDestination(world, start, destination);

    if (path.empty()) {
        qDebug() << "No path to the nearest health pack found.";
        return;
    }

    executePath(world, path);  // Follow the path
}

QStringList GameController::getAllCommands() const
{
    return commandMap.keys();
}

