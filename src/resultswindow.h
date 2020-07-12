#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QMainWindow>

namespace Ui {
class ResultsWindow;
}

class ResultsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResultsWindow(QWidget *parent = nullptr);
    ~ResultsWindow();

private:
    Ui::ResultsWindow *ui;
};

#endif // RESULTSWINDOW_H
