#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QKeyEvent>
#include <queue>
#include <pathfinder_class.h>
#include "myNode.h"
#include "overlaycontroller.h"
#include "levelingcontroller.h"

class View;

class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(std::shared_ptr<OverlayController> overlayController, std::shared_ptr<LevelingController> levelingController, QObject *parent = nullptr);
    void handleKeyInput(QKeyEvent *event);

    void handleSwitchScreen();
    void autoplay();
    static bool isGameOver;
    static bool isAutoPlay;
    static void GAMEOVER();
    QStringList getAllCommands() const;



public slots:
    void handleTextCommand(const QString& command); // Slot to handle text commands

private:
    QMap<QString, std::function<void()>> commandMap;  // Holds commands and their actions
    std::shared_ptr<OverlayController> overlayController;
    std::shared_ptr<LevelingController> levelingController;
    std::vector<int> planPath(int x, int y);
    int findNearestHealthPack(LevelData &world);
    void executeMoves();
    void processNextMove();
    std::queue<int> enemyQueue;
    std::queue<std::pair<int, int>> moveQueue;
    const int HEALTH_THRESHOLD = 30.0f;
    bool headingToHealthPack = false;
    std::queue<std::pair<int, int>> healthPackPathQueue;
    std::vector<int> findPathToDestination(LevelData &world, const Tile &start, const Tile &destination) ;
    void executePath(LevelData &world, const std::vector<int> &path) ;
    void initializeCommands();  // Populates the command map
    void navigateToNearestHealthPack();
    void navigateToNearestEnemy();
    void gotoXy();

    int goToIndexX;//for goToXY command
    int goToIndexY;//for goToXY command


    void clearQueue();
};

#endif // GAMECONTROLLER_H
