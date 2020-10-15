#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  ///QFile outFile("/home/helther/csv.csv"); /// todo temp
  /// for file in dirLogs
  /// make QVector<RaceLogInfo>
  ///DataSetParser::convertToCSV(vector of results, outFile);
  MainWindow mainW;
  mainW.show();
  return app.exec();
}
