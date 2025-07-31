#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QTextEdit>
#include <QLineEdit>
#include "view.h"
#include "levelingcontroller.h"

class TextView : public QTextEdit, public View {
    Q_OBJECT

public:
    explicit TextView(QWidget* parent = nullptr);

    // Override virtual methods from View
    void updateProtagonistPosition(int x, int y) override;
    QGraphicsScene* getScene() override;
    void createPoisonGasAnimation(int x, int y) override;
    void createExplosionAnimation(int x, int y) override;
    void updateHealthBar(int health) override;
    void updateEnergyBar(int energy) override;
    void hide() override;
    void handleHealthPackPickup(int xPos, int yPos)override;
    bool isRender(int x, int y) override;
    void visualizeAttack(Protagonist* protagonist)override;
    void visualizeHealthPackPickup(Protagonist* protagonist)override;
    void visualizeMovement(Protagonist* protagonist, int newX, int newY)override;
    void visualizePoisoned(Protagonist* protagonist)override;
    void visualizeDeath(Protagonist* protagonist)override;

    void initializeCompleter(const QStringList &commands);

    // Render the game state as a grid
    void render(LevelData& world)override;

    void setZoomValue(int newZoomValue);

    std::shared_ptr<QLineEdit> getCommandInput() const;

private:
    QString generateGrid(LevelData& world); // Helper to generate the grid as text
    QString grid;

   // Input field for commands
    std::shared_ptr<QLineEdit> commandInput;
    std::shared_ptr<QCompleter> completer;

    //QCompleter* completer
    //specific for text view
    int displayRows;
    int displayCols;
    int zoomValue=14;
    int startRow;
    int startCol;
    int endRow ;
    int endCol ;
    std::pair<int, int> protagonistGridPosition; // Initial position at (0, 0)
    bool wasPopupCompletion = false;

private slots:
    void processCommand(); // Slot to process input commands

signals:
    void commandEntered(const QString& command); // Signal to emit the command

};

#endif // TEXTVIEW_H
