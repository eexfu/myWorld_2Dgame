#include "mainwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QKeyEvent>
#include <QMovie>

#include "world.h"
#include "secondscreen.h"
#include "overlaycontroller.h"

#include "textview.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set "Health" label
    ui->textEdit->setText("Health");
    ui->textEdit->setAlignment(Qt::AlignCenter); // Center-align the text
    ui->textEdit->setReadOnly(true);            // Make it read-only
    ui->textEdit->setStyleSheet("background: transparent; border: none; font-weight: bold; font-size: 14px;");
    // Set "Energy" label
    ui->textEdit_2->setText("Energy");
    ui->textEdit_2->setAlignment(Qt::AlignCenter); // Center-align the text
    ui->textEdit_2->setReadOnly(true);            // Make it read-only
    ui->textEdit_2->setStyleSheet("background: transparent; border: none; font-weight: bold; font-size: 14px;");

    // Create the controllers
    graphicView = std::make_shared<GraphicView>(this);

    // Set up health and energy scenes for GraphicView
    QGraphicsScene* healthScene = new QGraphicsScene(this);
    graphicView->setHealthScene(healthScene);
    ui->graphicsView_2->setScene(healthScene);

    QGraphicsScene* energyScene = new QGraphicsScene(this);
    graphicView->setEnergyScene(energyScene);
    ui->graphicsView_3->setScene(energyScene);

    // Initialize TextView but don't display it yet
    textView = std::make_shared<TextView>(this);
    textView->setGeometry(ui->graphicsView->geometry());
    textView->hide();

    // Set up the overlay controller and add both views
    overlayController = std::make_shared<OverlayController>(graphicView.get());
    overlayController->addView(textView.get());

    // Initialize the leveling controller
    levelingController = std::make_shared<LevelingController>();

    // Initialize the game controller
    gameController = std::make_shared<GameController>(overlayController, levelingController);

    textView->initializeCompleter(gameController->getAllCommands());


    // Render the first level in GraphicView
    connect(textView.get(), &TextView::commandEntered, gameController.get(), &GameController::handleTextCommand);
    int healthBarBottom = ui->graphicsView_2->geometry().bottom();
    int margin = 40;

    textView->getCommandInput()->setParent(this);
    textView->getCommandInput()->move(
        ui->graphicsView_2->x(),
        healthBarBottom + margin
        );
    textView->getCommandInput()->resize(200, 30);  // pick a size you like

    // Make sure it's visible
    //textView->getCommandInput()->show();


    // graphicView->render(levelingController->getCurrentWorld());
    overlayController->renderCurrentView(levelingController->getCurrentWorld());

    // Set up the main scene
    ui->graphicsView->setScene(graphicView->getScene());

    connect(overlayController.get(), &OverlayController::enterDoor, this, &MainWindow::reRender);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    gameController->handleKeyInput(event); // Handle game logic updates

    switch (event->key()) {
        case Qt::Key_G: // Switch to another cView
            ui->graphicsView->setScene(overlayController->getCurrentScene()); // Reattach the scene for another View
            //overlayController->switchView(0);    // Switch to another View
            break;

        default:break;
    }
}




void MainWindow::zoomIn()
{

    int idx = overlayController->getCurrentViewIndex();
    if (idx == 0) {
        // GraphicView zoom
        ui->graphicsView->scale(1.1, 1.1);
        currentZoomFactor *= 1.1;
    } else {
       // std::cout<<"zoomed textview"<<std::endl;
        // TextView is a QTextEdit, so just call zoomIn(1)
        textView->setZoomValue(1);
        //overlayController->renderCurrentView(levelingController->getCurrentWorld());
        currentZoomFactor *= 1.1;

    }
}

void MainWindow::zoomOut()
{

    int idx = overlayController->getCurrentViewIndex();
    if (idx == 0)
    {
        if (currentZoomFactor > minZoomFactor) {
            ui->graphicsView->scale(0.9, 0.9);
            currentZoomFactor *= 0.9;
        }

    }else
    {

        //std::cout<<"zoomed textview"<<std::endl;
        // TextView is a QTextEdit, so just call zoomIn(1)
        textView->setZoomValue(-1);
        //overlayController->renderCurrentView(levelingController->getCurrentWorld());
        currentZoomFactor *= 0.9;

    }

}

void MainWindow::reRender(){
    levelingController->switchToLevel(levelingController->getCurrentLevel() + 1);
    overlayController->renderCurrentView(levelingController->getCurrentWorld());
    ui->graphicsView->setScene(overlayController->getCurrentScene());
}

MainWindow::~MainWindow()
{
    delete ui;
}
