#include <QCoreApplication>
#include <parser.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  QString testPath = "D:/Dev/PARSER_tests/";
  QString unixPath = "/mnt/Media/Dev/PARSER_tests/";
  QVector<QString> tests{ /*"P.xml", "Q.xml", "R.xml",*/ "t.rcd",
                          "t.veh",
                          "t.HDV" };
  for (const auto &test : tests)
  {
    Parser testParser = Parser(testPath + test);
    testParser.readFileContent();
  }

  // Parser::backupFile(unixPath + "P.xml", testPath);
  // Parser::restoreFile(unixPath + "testF.rcd", unixPath + "testB.rcd");
  // Parser p(unixPath + "writeTest.rcd");
  // p.writeModFile<int>("elem", 2, 3);

  return QCoreApplication::exec();
}
