#include <QApplication>
#include <parser.h>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

#ifdef QT_DEBUG
  /// main test

  QString testPath = "D:/Dev/PARSER_tests/";
  QString unixPath = "/mnt/Media/Dev/PARSER_tests/";
  QString currentPath = testPath;
  QVector<QString> tests{
    "t.rcd", "t.veh", "t.HDV", "P.xml", "Q.xml", "R.xml"
  }; /*
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
  /*
  try
  {
    DBHelper *testDB = new DBHelper();
    testDB->initDB();
    delete testDB;
    auto p = QFile(currentPath + tests[3]);
    auto q = QFile(currentPath + tests[4]);
    auto r = QFile(currentPath + tests[5]);
    auto pid = parseFile(PQXmlParser(p));
    auto qid = parseFile(PQXmlParser(q));
    auto rid = parseFile(RXmlParser(r));
    testDB = new DBHelper();
    int seasonId = testDB->addNewSeason("default season");
    testDB->addNewRace({ pid, qid, rid, seasonId, "Monza", 10 });
    testDB->viewTable(::DBTableNames::Races);
    testDB->viewTable(::DBTableNames::Sessions);
    //testDB->viewTable(::DBTableNames::RaceRes);
    delete testDB;
  } catch (std::exception &e)
  {
    qDebug() << e.what();
    QFile db("CMM.db3");
    if (db.exists()) db.remove();
    return -1;
  }
*/
#endif
  MainWindow mainW;
  mainW.show();
  return app.exec();
}
