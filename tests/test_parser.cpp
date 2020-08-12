#include "test_utility.h"
#include "../src/parser.h"

static QVector<QString> testFiles{ "P.xml",
                                   "Q.xml",
                                   "R.xml" /*"t.rcd", "t.veh", "t.HDV",*/ };
static QVector<QString> testFilesExpected{
  "PExpected.txt",
  "QExpected.txt",
  "RExpected.txt" /*"t.rcd", "t.veh", "t.HDV",*/
};

void testXMLFileRead()
{
  TEST_BEGIN("Parser readFile Test");

  auto p = QFile(TESTDATA_PATH + testFiles[0]);
  auto q = QFile(TESTDATA_PATH + testFiles[1]);
  auto r = QFile(TESTDATA_PATH + testFiles[2]);
  auto pParser = PQXmlParser(p);
  auto pData = pParser.getParseData();
  auto qParser = PQXmlParser(q);
  auto qData = qParser.getParseData();
  auto rParser = RXmlParser(r);
  auto rData = rParser.getParseData();
  QFile pFileE(TESTDATA_PATH + testFilesExpected[0]);
  QFile qFileE(TESTDATA_PATH + testFilesExpected[1]);
  QFile rFileE(TESTDATA_PATH + testFilesExpected[2]);
  pFileE.open(QIODevice::ReadWrite);
  qFileE.open(QIODevice::ReadWrite);
  rFileE.open(QIODevice::ReadWrite);
  QTextStream pstream(&pFileE);
  QTextStream qstream(&qFileE);
  QTextStream rstream(&rFileE);
  QString pString;
  QString qString;
  QString rString;
  pString << pData;
  qString << qData;
  rString << rData;
  auto pStringE = pstream.readAll();
  auto qStringE = qstream.readAll();
  auto rStringE = rstream.readAll();
  pFileE.close();
  qFileE.close();
  rFileE.close();
  ASSERT_EQUAL("Pracice log file", pString, pStringE);
  ASSERT_EQUAL("Quali log file", qString, qStringE);
  ASSERT_EQUAL("Race log file", rString, rStringE);

  TEST_END();
}

void testXMLParser() { testXMLFileRead(); }

int main()
{
  testXMLParser();
  return 0;
}

/*
/// backup restore test
 Parser::backupFile(currentPath + "P.xml", currentPath);
 Parser::restoreFile(currentPath + "testF.rcd", currentPath +
"testB.rcd");

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
