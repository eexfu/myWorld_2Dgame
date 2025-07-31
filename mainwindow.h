#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "secondscreen.h"
#include "graphicview.h"
#include "view.h"
#include "gamecontroller.h"
#include "textview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void zoomIn();
    void zoomOut();
    void reRender();

private:
    Ui::MainWindow *ui;
    SecondScreen *s2;
    std::shared_ptr<GameController> gameController;
    std::shared_ptr<OverlayController> overlayController;
    std::shared_ptr<LevelingController> levelingController;
    std::shared_ptr<GraphicView> graphicView;
    std::shared_ptr<TextView> textView;

    std::shared_ptr<World> world;

    const int STEP = 10;
    const int SIZE = 20;
    double currentZoomFactor = 1.0;  // Tracks the current zoom level
    const double maxZoomFactor = 3.0; // Maximum zoom in factor
    const double minZoomFactor = 0.2; // Minimum zoom out factor
};
#endif // MAINWINDOW_H
