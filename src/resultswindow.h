#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QWidget>

namespace Ui {
class resultswindow;
}

class resultswindow : public QWidget
{
    Q_OBJECT

public:
    explicit resultswindow(QWidget *parent = nullptr);
    ~resultswindow();

private:
    Ui::resultswindow *ui;
};

#endif // RESULTSWINDOW_H
