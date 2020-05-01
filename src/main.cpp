#include <QCoreApplication>
#include <parser.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  QString testPath = "D:/Dev/PARSER_tests/";
  QVector<QString> tests{
    "P.xml", "Q.xml", "R.xml", "t.rcd", "t.veh", "t.HDV"
  };
  for (const auto &test : tests)
  {
    Parser testParser = Parser(testPath + test);
    testParser.readFileContent();
  }

  Parser::backupFile(testPath + "P.xml", testPath);
  return QCoreApplication::exec();
}
