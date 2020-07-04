#include <QCoreApplication>
#include <parser.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  /// main test
  QString testPath = "D:/Dev/PARSER_tests/";
  QString unixPath = "/mnt/Media/Dev/PARSER_tests/";
  QString currentPath = testPath;
  QVector<QString> tests{ "t.rcd", "t.veh", "t.HDV" };
  for (const auto &test : tests)
  {
    auto file = QFile(currentPath + test);
    Parser testParser = Parser(file);
    testParser.readFileContent();
  }
  /// backup restore test
  // Parser::backupFile(currentPath + "P.xml", currentPath);
  // Parser::restoreFile(currentPath + "testF.rcd", currentPath + "testB.rcd");

  /// write file test
  /*
  QFile writefile(currentPath + "writeTest.rcd");
  writefile.open(QIODevice::WriteOnly);
  writefile.write("elem 2");
  writefile.close();
  Parser p(writefile);
  auto b = p.writeModFile<int>("elem", 2, 3);
  b = !b;
  writefile.remove();
    */
  return QCoreApplication::exec();
}
