#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QWidget>

namespace Ui {
class Resultswindow;
}

class Resultswindow : public QWidget
{
  Q_OBJECT

public:
  explicit Resultswindow(QWidget *parent = nullptr);
  ~Resultswindow();

private:
  Ui::Resultswindow *ui;
};

#endif// RESULTSWINDOW_H
