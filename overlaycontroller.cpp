#include "overlaycontroller.h"
#include "gamecontroller.h"
#include "view.h"
#include "xenemy.h"
#include "graphicview.h"
#include <iostream>
#include <climits>
#include <QTimer>

OverlayController::OverlayController(View* initialView, QWidget* parent)
    : QObject(parent) {
    // Add the initial view to the list of views
    views.push_back(initialView); // No ownership transfer since it's a raw pointer
    currentViewIndex=0;
}


void OverlayController::addView(View *view) {
    views.push_back(view);
}

void OverlayController::switchView() {
    views[currentViewIndex]->hide();
    if(currentViewIndex % 2 == 0) currentViewIndex++;
    else currentViewIndex--;
}

void OverlayController::renderCurrentView(LevelData& world) {
    if (!views.empty()) {
        std::cout << "New World Rendered " << std::endl;
        views[currentViewIndex]->render(world);
    }
}

QGraphicsScene* OverlayController::getCurrentScene() {
    return views[currentViewIndex]->getScene();
}

int OverlayController::getCurrentViewIndex() const
{
    return currentViewIndex;
}

void OverlayController::updateProtagonistPosition(LevelData &world, int dx, int dy){
    if(dx == 0 && dy == 0) return;
    int x = world.protagonist->getXPos();
    int y = world.protagonist->getYPos();

    int newX = x + dx;
    int newY = y + dy;

    std::cout << newX << " " << newY << " " << world.row << " " << world.col << " " << world.protagonist->getHealth() << " " << world.protagonist->getEnergy() << std::endl;

    // within boundary
    if (newX >= 0 && newY >= 0 && newX < world.row && newY < world.col) {

        // don't let energy go below 0
        if (world.protagonist->getEnergy() <= 0) {
            std::cout << "GAME OVER! Out of energy!" << std::endl;
            views[currentViewIndex]->visualizeDeath(world.protagonist.get());
            GameController::GAMEOVER();
            return;
        }

        // don't let health go below 0
        if (world.protagonist->getHealth() <= 0) {
            std::cout << "GAME OVER! Out of health!" << std::endl;
            views[currentViewIndex]->visualizeDeath(world.protagonist.get());
            GameController::GAMEOVER();
            return;
        }

        //isDoor
        if(world.door->getXPos() == newX && world.door->getYPos() == newY){
            std::cout << "Enter Door, level: " << world.door->getTargetLevel() << std::endl;
            emit enterDoor();
            return;
        }

        // isHealthPack
        for (auto& healthPack : world.healthPacks) {
            if (healthPack->getXPos() == newX && healthPack->getYPos() == newY) {

                std::cout << "Picked up a health pack!" << std::endl;
                std::cout << "HealthPack'value: " << healthPack->getValue() << std::endl;
                // if protagonist's health is already at 100, don't update health bar
                if (world.protagonist->getHealth() == 100) {
                    break;
                }
                else if (world.protagonist->getHealth() + healthPack->getValue() > 100) {
                    world.protagonist->setHealth(100);
                } else {
                    world.protagonist->setHealth(world.protagonist->getHealth() + healthPack->getValue());
                }
                // if protagonist's energy is already at 100, don't update health bar
                if (world.protagonist->getEnergy() == 100) {
                    break;
                }
                else if (world.protagonist->getEnergy() + healthPack->getValue() > 100) {
                    world.protagonist->setEnergy(100);
                } else {
                    world.protagonist->setEnergy(world.protagonist->getEnergy() + healthPack->getValue());
                }
                healthPack->setValue(-1);

                // update the view
                 views[0]->updateHealthBar(world.protagonist->getHealth());
                 views[0]->updateEnergyBar(world.protagonist->getEnergy());

                // Remove the health pack from logical layer
                 views[0]->handleHealthPackPickup(healthPack->getXPos(), healthPack->getYPos());
                 views[currentViewIndex]->visualizeHealthPackPickup(world.protagonist.get());
                 world.healthPacks.erase(std::remove(world.healthPacks.begin(), world.healthPacks.end(), healthPack), world.healthPacks.end()); // Remove logical
                 std::cout << "Done H" << std::endl;
                 break;
            }
        }

        // isEnemy
        for (auto& enemy : world.enemies) {
            if (enemy->getXPos() == newX && enemy->getYPos() == newY) {
                // check if enemy is defeated, if so, enemy shoudln't affect protagonist's health and energy
                if (enemy->getDefeated()) {
                    break;
                }
                auto hitEnemy = [&](bool isXEnemy, bool isPEnemy){
                    // compare the protagonist's and enemy's health and update accordingly
                    if (world.protagonist->getHealth() <= enemy->getValue()) {
                        world.protagonist->setHealth(0);
                        views[0]->updateHealthBar(world.protagonist->getHealth());
                        views[currentViewIndex]->visualizeDeath(world.protagonist.get());
                        GameController::GAMEOVER();
                    } else {
                        if(isXEnemy && enemy->getValue() <= 30) world.protagonist->setHealth(world.protagonist->getHealth() - enemy->getValue() * 1.5);
                        else world.protagonist->setHealth(world.protagonist->getHealth() - enemy->getValue());
                        views[0]->updateHealthBar(world.protagonist->getHealth());
                        // increase protagonist's energy by the enemy's value, but not exceeding 100
                        if (world.protagonist->getEnergy() + enemy->getValue() > 100) {
                            world.protagonist->setEnergy(100);
                        } else {
                            world.protagonist->setEnergy(world.protagonist->getEnergy() + enemy->getValue());
                        }
                        views[0]->updateEnergyBar(world.protagonist->getEnergy());
                        if(isPEnemy) {
                            const int totalTicks = 3;
                            const int damagePerTick = 5;
                            for (int i = 1; i <= totalTicks; ++i) {
                                QTimer::singleShot(i * 1000, [&]() {
                                    double currentHealth = world.protagonist->getHealth();
                                    double newHealth = currentHealth - damagePerTick;

                                    if (newHealth < 0.0) {
                                        newHealth = 0.0;
                                    }

                                    world.protagonist->setHealth(newHealth);
                                    views[0]->updateHealthBar(newHealth);

                                    // Optional: Handle protagonist death
                                    if (newHealth == 0.0) {
                                        views[currentViewIndex]->visualizeDeath(world.protagonist.get());
                                        GameController::GAMEOVER();
                                    }
                                });
                            }
                        }
                        enemy->setDefeated(true);
                    }
                };

                if (dynamic_cast<PEnemy*>(enemy.get())) {
                    std::cout << "Encountered a PEnemy!" << std::endl;
                    hitEnemy(false, true);
                    for(int i = -1; i <= 1; i++){
                        for(int j = -1; j <= 1; j++){
                            views[currentViewIndex]->createPoisonGasAnimation(enemy->getXPos() + i, enemy->getYPos() + j);
                            views[currentViewIndex]->visualizePoisoned(world.protagonist.get());
                        }
                    }
                }
                else if(dynamic_cast<XEnemy*>(enemy.get())) {
                    std::cout << "Encountered a XEnemy!" << std::endl;
                    hitEnemy(true, false);
                    views[currentViewIndex]->visualizeAttack(world.protagonist.get());
                    if(enemy->getDefeated()){
                        views[currentViewIndex]->createExplosionAnimation(enemy->getXPos(), enemy->getYPos());
                    }
                }
                else {
                    std::cout << "Encountered an Enemy!" << std::endl;
                    hitEnemy(false, false);
                    views[currentViewIndex]->visualizeAttack(world.protagonist.get());
                }
                std::cout << "enemy's value: " << enemy->getValue() << std::endl;
                std::cout << "protagonist's health: " << world.protagonist->getHealth() << std::endl;
                //views[currentViewIndex]->visualizeAttack(world.protagonist.get());
                break;
            }
        }

        // move
        std::cout << "Move start" << std::endl;

        auto tileVal = world.tiles[(x + dx) + (y + dy) * world.col]->getValue();
        //auto tileVal = 1;

        if(tileVal != std::numeric_limits<float>::infinity()){
           // if out of bounds for the text view
            if(views[currentViewIndex]->isRender(newX,newY)){
                views[currentViewIndex]->render(world);
            }
            views[currentViewIndex]->visualizeMovement(world.protagonist.get(), newX, newY);
            views[currentViewIndex]->updateProtagonistPosition(newX, newY);
            // update the energy bar by substracting the tile value from the energy
            world.protagonist->setEnergy(world.protagonist->getEnergy() - tileVal/10);
            views[0]->updateEnergyBar(world.protagonist->getEnergy());
            if(dx != 0) world.protagonist->setXPos(newX);
            if(dy != 0) world.protagonist->setYPos(newY);
        }
        else{
            std::cout << "boundary" << std::endl;
        }

        std::cout << "Move end" << std::endl;
    } else {
        std::cout << "Out of bounds!" << std::endl;
    }
}
