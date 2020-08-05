#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  MainWindow mainW;
  mainW.show();
  return app.exec();
}
