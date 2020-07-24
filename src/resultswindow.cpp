#include "resultswindow.h"
#include "ui_resultswindow.h"

resultswindow::resultswindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::resultswindow)
{
    ui->setupUi(this);
}

resultswindow::~resultswindow()
{
    delete ui;
}
