#include "resultswindow.h"
#include "ui_resultswindow.h"

Resultswindow::Resultswindow(QWidget *parent)
  : QWidget(parent), ui(new Ui::Resultswindow)
{
  ui->setupUi(this);
}

Resultswindow::~Resultswindow() { delete ui; }
