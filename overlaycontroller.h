#ifndef OVERLAYCONTROLLER_H
#define OVERLAYCONTROLLER_H

#include <QObject>
#include <QWidget>
#include <vector>
#include "view.h"
#include "levelingcontroller.h"

class OverlayController : public QObject {
    Q_OBJECT

public:
    explicit OverlayController(View* initialView, QWidget* parent = nullptr);

    void addView(View * view);      // Add a new view
    void switchView();                   // Switch to a specific view
    void updateProtagonistPosition(LevelData& world, int dx, int dy); // Handle movement logic
    void renderCurrentView(LevelData& world);      // Render the active view
    QGraphicsScene* getCurrentScene();

    int getCurrentViewIndex() const;

signals:
    void enterDoor();

private:
    std::vector<View*> views;     // Stores multiple views
    int currentViewIndex = 0;                     // Index of the active view
};

#endif // OVERLAYCONTROLLER_H
