#include <QCoreApplication>
#include <parser.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Parser testParser = Parser("./P.xml");
    testParser.getFileData();
    //parser::pXMLLog("./R.xml");
    //parser::pXMLLog("./Q.xml");
    //parser::pXMLLog("./P.xml");
    //parser::GetFileType("./P.xml");
    //parser::backupFile("./P.xml","./", "_b");
    //parser::backupFile("/home/helther/P.xml", "/home/helther/");
    return a.exec();
}
