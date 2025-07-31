#include "secondscreen.h"
#include "ui_secondscreen.h"

SecondScreen::SecondScreen(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SecondScreen)
{
    ui->setupUi(this);
}

SecondScreen::~SecondScreen()
{
    delete ui;
}
