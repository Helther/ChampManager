#include "test_utility.h"
#include "../src/parser.h"
#include <QDebug>


const QVector<QString> testFiles{ "P.xml",
                                  "Q.xml",
                                  "R.xml" /*"t.rcd", "t.veh", "t.hdv",*/ };
const QVector<QString> testFilesExpected{
  "PExpected.txt",
  "QExpected.txt",
  "RExpected.txt" /*"t.rcd", "t.veh", "t.hdv",*/
};

const QString testDataSetExpected = "testE.csv";

const QString testHDVFile = "TestHDV.hdv";
const QString testINIFile = "TestUpgrades.ini";
const QString testINIFileExp = "TestUpgradesExpected.ini";
const QString testVEHFile = "TestVEH.veh";
const QString testRCDFile = "TestRCD.rcd";
const QString testVEHFileExp = "TestVEHExpected.veh";
const QString testRCDFileExp = "TestRCDExpected.rcd";


void testXMLFileRead()
{
  TEST_BEGIN("Parser XML readFile Test");

  auto p = QFile(TESTDATA_PATH + testFiles[0]);
  auto q = QFile(TESTDATA_PATH + testFiles[1]);
  auto r = QFile(TESTDATA_PATH + testFiles[2]);
  auto pParser = PXmlParser(p);
  auto pData = pParser.getParseData();
  auto qParser = QXmlParser(q);
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
  ASSERT_EQUAL("Pracice log file", pString.simplified(), pStringE.simplified());
  ASSERT_EQUAL("Quali log file", qString.simplified(), qStringE.simplified());
  ASSERT_EQUAL("Race log file", rString.simplified(), rStringE.simplified());
  TEST_END();
}

void testDataSetParser()
{
  TEST_BEGIN("DaTaSet Parser csv creation Test");
  auto p = QFile(TESTDATA_PATH + testFiles[0]);
  auto q = QFile(TESTDATA_PATH + testFiles[1]);
  auto r = QFile(TESTDATA_PATH + testFiles[2]);
  QVector<QPair<QString, RaceLogInfo>> dataSet;
  DataSetParser pParser(p);
  auto data = pParser.readXMLLog(seqElems::DataSetPQElems);
  dataSet.push_back({ getFileTypeById(pParser.getFileType()), data });
  DataSetParser qParser(q);
  data = qParser.readXMLLog(seqElems::DataSetPQElems);
  dataSet.push_back({ getFileTypeById(qParser.getFileType()), data });
  DataSetParser rParser(r);
  data = rParser.readXMLLog(seqElems::DataSetRElems);
  dataSet.push_back({ getFileTypeById(rParser.getFileType()), data });
  QFile outFile(TESTDATA_PATH + QString{ "test.csv" });
  DataSetParser::convertToCSV(dataSet, outFile);
  QFile eFile(TESTDATA_PATH + testDataSetExpected);
  eFile.open(QIODevice::ReadOnly);
  outFile.open(QIODevice::ReadOnly);
  QTextStream eStream(&eFile);
  QTextStream testStream(&outFile);
  const auto eString = eStream.readAll();
  const auto testString = testStream.readAll();
  outFile.close();
  outFile.remove();
  eFile.close();
  ASSERT_EQUAL("DataSet parsing", eString.simplified(), testString.simplified());
  TEST_END();
}

void testXMLParser() { testXMLFileRead(); }

void testFileBackupRestore()
{
  TEST_BEGIN("Parser backup/restore test");
  const QString testFilePath = TESTDATA_PATH + testFiles[0];
  const QString backupFilePath = TESTDATA_PATH + QString("backup.xml");
  QFile testFile(testFilePath);
  QFile backupFile(backupFilePath);
  // setup backupfile
  testFile.open(QIODevice::ReadOnly);
  backupFile.open(QIODevice::WriteOnly);
  backupFile.write(testFile.readAll());
  backupFile.close();
  testFile.close();

  auto result = Parser::backupFile(backupFilePath, TESTDATA_PATH);
  if (!result.result)
  {
    backupFile.remove();
    ASSERT_FAIL("Backup failed");
  }
  auto newBackupFile = QFile(result.fileFullPath);
  // change original file contents
  backupFile.open(QIODevice::ReadWrite);
  const QString original = backupFile.readAll();
  backupFile.resize(0);
  backupFile.write("test");
  backupFile.close();

  if (!Parser::restoreFile(backupFilePath, result.fileFullPath))
  {
    backupFile.remove();
    newBackupFile.remove();
    ASSERT_FAIL("Restore failed");
  }
  backupFile.open(QIODevice::ReadOnly);
  const QString restored = backupFile.readAll();
  backupFile.close();
  newBackupFile.remove();
  backupFile.remove();
  ASSERT_EQUAL("File content compare", original, restored);

  TEST_END();
}

void testHDVParser()
{
  TEST_BEGIN("HDV Parser Test");

  auto file = QFile(TESTDATA_PATH + testHDVFile);
  auto parser = HDVParser(file);
  HDVParams data = parser.getParseData();
  QString result;
  result << data;
  //qDebug() << result;

  TEST_END();
}

void testINIParser()
{
  QString filePath = TESTDATA_PATH + testINIFile;

  {
    TEST_BEGIN("INI Parser Test");
    auto file = QFile(filePath);
    auto parser = INIParser(file);
    QString result;
    result << parser.getParseData();
    //qDebug() << result;
    qDebug() << Qt::endl << "Missing entries:" << Qt::endl;
    result.clear();
    result << parser.getMissingData();
    //qDebug() << result;

    TEST_END();
  }

  TEST_BEGIN("INI Parser modify and write data Test");
  // modify test file and compare it's contents with expected file result
  auto result = Parser::backupFile(filePath, TESTDATA_PATH);
  if (!result.result)
  {
    ASSERT_FAIL("Backup failed");
  }

  auto fileE = QFile(TESTDATA_PATH + testINIFileExp);
  fileE.open(QIODevice::ReadOnly);
  QTextStream streamE(&fileE);
  streamE.seek(0);
  QString strE = streamE.readAll();

  auto file = QFile(result.fileFullPath);
  auto parser = INIParser(file);

  parser.modifyEntry(HDVEntries::FWSetting, QString::number(10));
  parser.modifyEntry(HDVEntries::BodyDragBase, QString::number(0.3));
  parser.modifyEntry(HDVEntries::FWLiftParams, QString::number(-0.1));
  parser.modifyEntry(HDVEntries::GeneralPowerMult, QString::number(1.5));
  parser.writeChanges();

  QTextStream stream(&file);
  stream.seek(0);
  QString str = stream.readAll();
  file.close();
  file.remove();
  ASSERT_EQUAL("INI File Comparison", strE, str);

  TEST_END();
}

void testHDVAndINIParser()
{
  TEST_BEGIN("Missing ini Entries hdv Parse Test");

  QString INIPath = TESTDATA_PATH + testINIFile;
  QString HDVPath = TESTDATA_PATH + testHDVFile;
  auto iniFile = QFile(INIPath);
  auto hdvFile = QFile(HDVPath);
  auto iniParser = INIParser(iniFile);
  auto hdvParser = HDVParser(hdvFile, iniParser.getMissingData());
  if (iniParser.getMissingData().size() != hdvParser.getParseData().size())
    ASSERT_FAIL("Parsers data entries are different");

  TEST_END();
}

void testRCDParser()
{
  QString filePath = TESTDATA_PATH + testRCDFile;
  {
    TEST_BEGIN("RCD Parser Test");
    auto file = QFile(filePath);
    auto parser = RCDParser(file);
    QString result;
    result << parser.getParseData();
    //qDebug() << result;
    //qDebug() << Qt::endl << "Missing entries:" << Qt::endl;

    TEST_END();
  }

  TEST_BEGIN("RCD Parser modify and write data Test");

  auto result = Parser::backupFile(filePath, TESTDATA_PATH);
  if (!result.result)
  {
    ASSERT_FAIL("Backup failed");
  }

  auto fileE = QFile(TESTDATA_PATH + testRCDFileExp);
  fileE.open(QIODevice::ReadOnly);
  QTextStream streamE(&fileE);
  streamE.seek(0);
  QString strE = streamE.readAll();

  auto file = QFile(result.fileFullPath);
  auto parser = RCDParser(file);

  parser.removeEntry("Nicholas Latifi");
  parser.addDriver("Nicholas Goatifi");
  parser.modifyEntry("Nicholas Goatifi", RCDEntries::Composure, 0.);
  parser.writeChanges();

  QTextStream stream(&file);
  stream.seek(0);
  QString str = stream.readAll();
  file.close();
  file.remove();
  ASSERT_EQUAL("RCD File Comparison", strE.simplified(), str.simplified());

  TEST_END();
}

void testVEHParser()
{
  QString filePath = TESTDATA_PATH + testVEHFile;

  {
    TEST_BEGIN("VEH Parser Test");
    auto file = QFile(filePath);
    auto parser = VEHParser(file);
    //qDebug() << getVEHDataString(parser.getParseData());

    TEST_END();
  }

  TEST_BEGIN("VEH Parser modify and write data Test");

  auto result = Parser::backupFile(filePath, TESTDATA_PATH);
  if (!result.result)
  {
    ASSERT_FAIL("Backup failed");
  }

  auto fileE = QFile(TESTDATA_PATH + testVEHFileExp);
  fileE.open(QIODevice::ReadOnly);
  QTextStream streamE(&fileE);
  streamE.seek(0);
  QString strE = streamE.readAll();

  auto file = QFile(result.fileFullPath);
  auto parser = VEHParser(file);

  parser.modifyDriverName("Nico Hulkenberg");
  parser.writeChanges();

  QTextStream stream(&file);
  stream.seek(0);
  QString str = stream.readAll();
  file.close();
  file.remove();
  ASSERT_EQUAL("VEH File Comparison", strE.simplified(), str.simplified());

  TEST_END();
}


int main()
{
  testXMLParser();
  testDataSetParser();
  testFileBackupRestore();
  testHDVParser();
  testINIParser();
  testHDVAndINIParser();
  testRCDParser();
  testVEHParser();
  if (true)
    ASSERT_FAIL("oops");
  return 0;
}

/*
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
