#include <QCoreApplication>
#include <parser.h>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
#ifdef QT_DEBUG
  /// main test

  QString testPath = "D:/Dev/PARSER_tests/";
  QString unixPath = "/mnt/Media/Dev/PARSER_tests/";
  QString currentPath = unixPath; /*
   QVector<QString> tests{ "t.rcd", "t.veh", "t.HDV", "P.xml", "Q.xml",
   "R.xml"}; for (const auto &test : tests)
   {
       Parser* testParser = nullptr;
       try {
           auto file = QFile(currentPath + test);
           testParser = new Parser(file);
       } catch (std::exception& e) {
        throw std::runtime_error(QString("\nCaught somethong ").toStdString() +
   e.what() + '\n'); delete testParser; continue;
       }
       testParser->readFileContent();
   }
   /// backup restore test
   // Parser::backupFile(currentPath + "P.xml", currentPath);
   // Parser::restoreFile(currentPath + "testF.rcd", currentPath + "testB.rcd");
 */
  /// write file test

  QFile writefile(currentPath + "writeTest.rcd");
  writefile.open(QIODevice::WriteOnly);
  writefile.write("some text<elem> 2 </elem>\n<elem2> 4 </elem2>\n");
  writefile.close();
  Parser p(writefile);
  QVector<WriteDataInput<int>> data{ { "elem", 2, 3 }, { "elem2", 4, 5 } };
  p.updateModFileData<int>(data);
  writefile.remove();

#endif
  return QCoreApplication::exec();
}
