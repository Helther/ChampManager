#include <QCoreApplication>
#include <parser.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString testPath = "D:/Dev/PARSER_tests/";
    QVector<QString> tests{"HDV_guide","Q.xml","R.xml"};
    for(const auto& test : tests)
    {
        Parser testParser = Parser(testPath + test);
        testParser.readFileContent();
    }
    //parser::pXMLLog("./R.xml");
    //parser::pXMLLog("./Q.xml");
    //parser::pXMLLog("./P.xml");
    //parser::GetFileType("./P.xml");
    //parser::backupFile("./P.xml","./", "_b");
    //parser::backupFile("/home/helther/P.xml", "/home/helther/");
    return a.exec();
}
