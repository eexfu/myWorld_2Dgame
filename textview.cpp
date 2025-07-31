#include "textview.h"
#include <iostream>
#include <ostream>
#include <QTimer>
#include<QCompleter>

TextView::TextView(QWidget* parent)
    : QTextEdit(parent) {
    setReadOnly(true);
    setStyleSheet("font-family: Consolas; font-size: 15px; background-color: black; color: white;");

    commandInput = std::make_shared<QLineEdit>(this);
    commandInput->setPlaceholderText("Enter command (up, down, left, right)");
    commandInput->setStyleSheet("background-color: white; color: black; font-size: 12px;");

   // commandInput->setGeometry(800,0,200,50); // Position at the bottom
    connect(commandInput.get(), &QLineEdit::returnPressed, this, &TextView::processCommand);

//    append("TextView Initialized: Ready to visualize game state.");
}

void TextView::updateProtagonistPosition(int newX, int newY) {
    // Calculate the old and new relative positions in the visible grid
    int protagonistOldX = protagonistGridPosition.first;
    int protagonistOldY = protagonistGridPosition.second;

    int protagonistNewX = newX - startCol;
    int protagonistNewY = newY - startRow;

    // Calculate the index in the QString for the old and new positions
    int oldIndex = (protagonistOldY * (displayCols + 1)) + protagonistOldX; // +1 accounts for the '\n'
    int newIndex = (protagonistNewY * (displayCols + 1)) + protagonistNewX;

    // Ensure indices are valid
    if (oldIndex >= 0 && oldIndex < grid.size() && newIndex >= 0 && newIndex < grid.size()) {
        // Clear the old position
        if (grid[oldIndex] == 'P') {
            grid[oldIndex] = '.'; // Reset the old position to empty
        }

        // Set the new position
        grid[newIndex] = 'P'; // Place the protagonist in the new position

        // Update the protagonist's stored position
        protagonistGridPosition = {protagonistNewX, protagonistNewY};

        // Refresh the grid display
        setPlainText(grid); // Replace the content with the updated grid
    } else {
        // Log an error if the indices are invalid
        append(QString("Invalid move: New position (%1, %2) is out of bounds.")
                   .arg(newX)
                   .arg(newY));
    }
}
bool TextView::isRender(int x, int y){

    return !(x >= startCol && x < endCol && y >= startRow && y < endRow);
}

QGraphicsScene* TextView::getScene() {
    return nullptr; // Not applicable for TextView
}

void TextView::createPoisonGasAnimation(int x, int y) {
    append(QString("Poison gas animation triggered at (%1, %2).").arg(x).arg(y));
}

void TextView::createExplosionAnimation(int x, int y){
    append(QString("Explosion animation triggered at (%1, %2).").arg(x).arg(y));
}

void TextView::updateHealthBar(int health) {
    append(QString("Protagonist health updated: %1").arg(health));
}

void TextView::updateEnergyBar(int energy) {
    append(QString("Protagonist energy updated: %1").arg(energy));
}

void TextView::render(LevelData& world) {

    clear(); // Clear the previous grid
    show();
    commandInput->show();
    int protagonistX = world.protagonist->getXPos();
    int protagonistY = world.protagonist->getYPos();
    protagonistGridPosition = qMakePair(protagonistX - startCol, protagonistY - startRow);

    std::cout << "Text view " << std::endl;

   // append(); // Append the new grid
    setPlainText(generateGrid(world));

    //append(QString("Protagonist Health: %1").arg(world.protagonist->getHealth()));
   // append(QString("Protagonist Energy: %1").arg(world.protagonist->getEnergy()));
}
QString TextView::generateGrid(LevelData& world) {

    grid.clear();

    displayRows =  world.row;
    displayCols =  world.col;

    int protagonistX = world.protagonist->getXPos();
    int protagonistY = world.protagonist->getYPos();

    startRow =0;
    startCol =0;
    endRow =displayRows;
    endCol = displayCols;

    std::vector<std::vector<char>> gridData(endRow - startRow, std::vector<char>(endCol - startCol, '.'));

    for (int y = startRow; y < endRow; ++y) {
        for (int x = startCol; x < endCol; ++x) {
            auto& tile = world.tiles[x + y * world.col];
            if (tile->getValue() == std::numeric_limits<float>::infinity()) {
                gridData[y - startRow][x - startCol] = '#';
            }
        }
    }


    for (const auto& healthPack : world.healthPacks) {
        if (healthPack->getYPos() >= startRow && healthPack->getYPos() < endRow && healthPack->getXPos() >= startCol && healthPack->getXPos() < endCol) {
            if(healthPack->getValue()==-1)
            {
                gridData[healthPack->getYPos() - startRow][healthPack->getXPos() - startCol] = '.';

            }
            else
            {
             gridData[healthPack->getYPos() - startRow][healthPack->getXPos() - startCol] = 'H';

            }
        }
    }
     gridData[ world.door->getYPos()- startRow][world.door->getXPos() - startCol] = 'D';



    for (const auto& enemy : world.enemies) {
        if (enemy->getYPos() >= startRow && enemy->getYPos() < endRow && enemy->getXPos() >= startCol && enemy->getXPos() < endCol) {

            if(enemy->getDefeated())
            {
                gridData[enemy->getYPos() - startRow][enemy->getXPos() - startCol] = '.';

            }
            else if (dynamic_cast<PEnemy*>(enemy.get())) {
                gridData[enemy->getYPos() - startRow][enemy->getXPos() - startCol] = 'X';
            } else {
                gridData[enemy->getYPos() - startRow][enemy->getXPos() - startCol] = 'E';
            }
        }
    }

    if (protagonistY >= startRow && protagonistY < endRow && protagonistX >= startCol && protagonistX < endCol) {
        gridData[protagonistY - startRow][protagonistX - startCol] = 'P';
        protagonistGridPosition = qMakePair(protagonistX - startCol, protagonistY - startRow);


    }

    for (int i = 0; i < endRow - startRow; ++i) {
        for (int j = 0; j < endCol - startCol; ++j) {
            grid.append(gridData[i][j]);
        }
        grid.append("\n");
    }

    return grid;
}

std::shared_ptr<QLineEdit> TextView::getCommandInput() const
{
    return commandInput;
}

void TextView::setZoomValue(int newZoomValue)
{
    zoomValue+=newZoomValue;  // Example
    QString poisonStyle = QString(
   "font-family: Consolas; font-size: %1px;background-color: black; color: white;"
                              ).arg(zoomValue);

    setStyleSheet(poisonStyle);

}

void TextView::hide() {
    commandInput->hide();
    clear();
    QTextEdit::hide(); // Hide the text view
}

void TextView::handleHealthPackPickup(int xPos, int yPos) {
    append(QString("Health pack picked up at (%1, %2).").arg(xPos).arg(yPos));
}

void TextView::processCommand() {
    QString command = commandInput->text().trimmed().toLower(); // Get and normalize the input

    if (command == "help")
    {

        wasPopupCompletion = (completer->completionMode() == QCompleter::PopupCompletion);
        completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        // or QCompleter::PopupCompletion

        // 2. Force QCompleter to show a full list
        completer->setCompletionPrefix(QString());
        completer->complete();

        // Keep "help" in the line
        commandInput->setText("help");

        // 2) Optionally also clear the input or keep "help"




    }else{

    commandInput->clear(); // Clear the input box

    emit commandEntered(command); // Emit the signal with the command
    }
}

void TextView::visualizeAttack([[maybe_unused]]Protagonist* protagonist) {
    // Ensure the protagonist index is valid
    int protagonistIndex = (protagonistGridPosition.second * (displayCols + 1)) + protagonistGridPosition.first;
    if (protagonistIndex < 0 || protagonistIndex >= grid.size()) {
        append("Error: Protagonist index out of bounds.");
        return;
    }

    QString attackStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: red;"
                              ).arg(zoomValue);

    // Temporarily change the protagonist's color to red
   // QString  attackStyle= "font-family: Consolas; font-size: 12px; background-color: black; color: red;";

    QString normalStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: white;"
                              ).arg(zoomValue);

   // QString normalStyle = "font-family: Consolas; font-size: 12px; background-color: black; color: white;";
    setStyleSheet(attackStyle);
    std::cout<<"Attack enemy dtected+++++"<<std::endl;


    // Use a static flag to prevent multiple overlapping resets
    static bool resettingColor = false;
    if (!resettingColor) {
        resettingColor = true;

        // Restore the original color after a short delay
        QTimer::singleShot(500, this, [=]() {
            setStyleSheet(normalStyle);
            resettingColor = false;
        });
    }

    // Refresh the grid display
    setPlainText(grid);
}

void TextView::visualizeHealthPackPickup(Protagonist* protagonist) {
    int protagonistIndex = (protagonistGridPosition.second * (displayCols + 1)) + protagonistGridPosition.first;
    if (protagonistIndex < 0 || protagonistIndex >= grid.size()) {
        append("Error: Protagonist index out of bounds.");
        return;
    }

    // Temporarily change the protagonist's color to red

    //QString attackStyle = "font-family: Consolas; font-size: 12px; background-color: black; color: green;";

    QString attackStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: green;"
                              ).arg(zoomValue);
    //QString normalStyle = "font-family: Consolas; font-size: 12px; background-color: black; color: green;";

    QString normalStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: white;"
                              ).arg(zoomValue);

    setStyleSheet(attackStyle);

    // std::cout << "Visiual *****************************************" << std::endl;

    // Use a static flag to prevent multiple overlapping resets
    static bool resettingColor = false;
    if (!resettingColor) {
        resettingColor = true;

        // Restore the original color after a short delay
        QTimer::singleShot(500, this, [=]() {
            setStyleSheet(normalStyle);
            resettingColor = false;
        });
    }

    // Refresh the grid display
    setPlainText(grid);
}

void TextView::visualizeMovement(Protagonist* protagonist, int newX, int newY) {
    int protagonistOldX = protagonistGridPosition.first;
    int protagonistOldY = protagonistGridPosition.second;
    int protagonistNewX = newX;
    int protagonistNewY = newY;

    // Get the index of the protagonist's old and new position
    int oldIndex = (protagonistOldY * (displayCols + 1)) + protagonistOldX;
    int newIndex = (protagonistNewY * (displayCols + 1)) + protagonistNewX;

    // Temporarily mark the old position as a trail
    if (oldIndex >= 0 && oldIndex < grid.size()) {
        grid[oldIndex] = '-';  // Mark trail
    }

    // Set the new position
    if (newIndex >= 0 && newIndex < grid.size()) {
        grid[newIndex] = 'P';  // Set protagonist's new position
    }

    // Refresh the grid to show the movement and trail
    setPlainText(grid);

    // Wait for a short period and then erase the trail
    QTimer::singleShot(500, this, [=]() {
        if (oldIndex >= 0 && oldIndex < grid.size()) {
            grid[oldIndex] = '.';  // Reset trail back to empty
        }
        setPlainText(grid);  // Refresh the grid
    });
}

void TextView::visualizePoisoned(Protagonist* protagonist) {
    int protagonistIndex = (protagonistGridPosition.second * (displayCols + 1)) + protagonistGridPosition.first;
    if (protagonistIndex < 0 || protagonistIndex >= grid.size()) {
        append("Error: Protagonist index out of bounds.");
        return;
    }

    //QString poisonStyle = "font-family: Consolas; font-size: 12px; background-color: black; color: purple;";
    QString poisonStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: purple;"
                              ).arg(zoomValue);


    //QString normalStyle = "font-family: Consolas; font-size: 12px; background-color: black; color: white;";

    QString normalStyle = QString(
                              "font-family: Consolas; font-size: %1px;background-color: black; color: white;"
                              ).arg(zoomValue);

    std::cout<<"poison enemy dtected+++++"<<std::endl;

    setStyleSheet(poisonStyle);

    static bool resettingColor = false;
    if (!resettingColor) {
        resettingColor = true;

        QTimer::singleShot(500, this, [=]() {
            setStyleSheet(normalStyle);
            resettingColor = false;
        });
    }
    setPlainText(grid);


}

void TextView::visualizeDeath(Protagonist* protagonist) {
    int protagonistIndex = (protagonistGridPosition.second * (displayCols + 1))
    + protagonistGridPosition.first;

    // Basic boundary check
    if (protagonistIndex < 0 || protagonistIndex >= grid.size()) {
        append("Error: Protagonist index out of bounds in visualizeDeath.");
        return;
    }

    // Replace 'P' with 'R'
    grid[protagonistIndex] = 'R';

    // Compute the row boundaries.
    // rowStart is the index of the first character in that row;
    // rowEnd is the last valid character in that row (before the newline).
    int rowStart = protagonistGridPosition.second * (displayCols + 1);
    int rowEnd   = rowStart + displayCols - 1; // because rowEnd is the last valid char (not the '\n')

    // Write the 'I' if it doesn't cross the row boundary or string end
    if ((protagonistIndex + 1) <= rowEnd && (protagonistIndex + 1) < grid.size()) {
        grid[protagonistIndex + 1] = 'I';
    }

    // Write the 'P' if it doesn't cross the row boundary or string end
    if ((protagonistIndex + 2) <= rowEnd && (protagonistIndex + 2) < grid.size()) {
        grid[protagonistIndex + 2] = 'P';
    }

    // Update the text view
    setPlainText(grid);
    append("Game Over: The protagonist has died.");
}

void TextView::initializeCompleter(const QStringList &commands)
{
    // Create a QCompleter from the list of command strings
    completer = std::make_unique<QCompleter>(commands, this);

    // Case-insensitive completion
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);


    // Attach it to our commandInput
    commandInput->setCompleter(completer.get());

    connect(completer.get(), QOverload<const QString &>::of(&QCompleter::activated),
            this, [this](const QString &completedText) {
                // This is triggered when the user selects an item from the popup
                qDebug() << "User picked completion:" << completedText;

                // If you had forcibly switched to popup mode (e.g. for 'help'), revert now:
                completer->setCompletionMode(QCompleter::InlineCompletion);
            });
}
