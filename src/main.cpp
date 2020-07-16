#include <QApplication>
#include <parser.h>
#include <dbhelper.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
#ifdef QT_DEBUG /*
  /// main test

  QString testPath = "D:/Dev/PARSER_tests/";
  QString unixPath = "/mnt/Media/Dev/PARSER_tests/";
  QString currentPath = unixPath;
  QVector<QString> tests{
    "t.rcd", "t.veh", "t.HDV", "P.xml", "Q.xml", "R.xml"
  };
  Parser *testParser = nullptr;
  try
  {
    auto file = QFile(currentPath + tests[0]);
    testParser = new RCDParser(file);
    testParser->readFileContent();
    delete testParser;
    auto veh = QFile(currentPath + tests[1]);
    testParser = new VEHParser(veh);
    testParser->readFileContent();
    delete testParser;
    auto hdv = QFile(currentPath + tests[2]);
    testParser = new HDVParser(hdv);
    testParser->readFileContent();
    delete testParser;
    auto p = QFile(currentPath + tests[3]);
    testParser = new PQXmlParser(p);
    testParser->readFileContent();
    delete testParser;
    auto q = QFile(currentPath + tests[4]);
    testParser = new PQXmlParser(q);
    testParser->readFileContent();
    delete testParser;
    auto r = QFile(currentPath + tests[5]);
    testParser = new RXmlParser(r);
    testParser->readFileContent();
    delete testParser;
  } catch (std::exception &e)
  {
    throw std::runtime_error(QString("\nCaught somethong ").toStdString()
                             + e.what() + '\n');
  }

  /// backup restore test
  // Parser::backupFile(currentPath + "P.xml", currentPath);
  // Parser::restoreFile(currentPath + "testF.rcd", currentPath +
  //"testB.rcd");

  /// write file test

  QFile writefile(currentPath + "writeTest.rcd");
  writefile.open(QIODevice::WriteOnly);
  writefile.write("some text<elem> 2 </elem>\n<elem2> 4 </elem2>\n");
  writefile.close();
  RCDParser p(writefile);
  QVector<WriteDataInput<int>> data{ { "elem", 2, 3 }, { "elem2", 4, 5 } };
  p.updateModFileData<int>(data);
  writefile.remove();
*/
  DBHelper *testDB = new DBHelper();
  testDB->initDB();
  delete testDB;
  QFile db("./CMM.db3");
  if (db.exists()) db.remove();
#endif
  return app.exec();
}
