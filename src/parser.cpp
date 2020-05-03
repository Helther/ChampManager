#include <parser.h>

#include <QDateTime>
#include <QXmlStreamReader>
//#ifdef QT_DEBUG QT_NO_DEBUG_OUTPUT
//#endif
using namespace parserConsts;

Parser::Parser(const QString &FileName) : fileName(FileName)
{
  QFile file(FileName);
  if (openFile(file, QIODevice::ReadWrite))
  {
    auto buffer = file.readAll();
    fileData = QString(buffer).toUtf8();
    if (!hasError) fileType = readFileType();
  }
}

Parser::~Parser()
{
  QFile file(fileName);
  if (file.isOpen()) file.close();
}

FileType Parser::readFileType()
{
  const auto tokenName = "FixedUpgrades";// elem prior to one needed
  // find log type token
  QXmlStreamReader xml(fileData);
  if (!xml.readNextStartElement())// is file xml file
  {
    qDebug() << "file not xml\n";
    return readModFileType();
  }
  findXMLElement(xml, tokenName);
  for (int i = 0; i < 4; ++i)// x4 to get to type token
  {
    xml.readNext();
    if (xml.hasError())
    {
      raiseError(xml.errorString());
      return FileType::Error;
    }
  }
  if (xml.name() == typeName::Racetype)
  {
    qDebug() << "Race\n";
    return FileType::RaceLog;
  }
  if (xml.name() == typeName::Qualtype)
  {
    qDebug() << "Quali\n";
    return FileType::QualiLog;
  }
  if (xml.name().contains(typeName::Practtype))
  {
    qDebug() << "Practice\n";
    return FileType::PracticeLog;
  }
  raiseError("No valid log type was found");
  return FileType::Error;
}

FileType Parser::readModFileType()
{
  QFile file(fileName);
  auto fileinfo = file.fileName().split(".", QString::KeepEmptyParts);
  const QString &fileExt = fileinfo.at(fileinfo.size() - 1);
  if (fileExt == typeName::rcdFile)
  {
    qDebug() << "rcd\n";
    return FileType::RCD;
  }
  if (fileExt == typeName::hdvFile)
  {
    qDebug() << "hdv\n";
    return FileType::HDV;
  }
  if (fileExt == typeName::vehFile)
  {
    qDebug() << "veh\n";
    return FileType::VEH;
  }
  raiseError("No valid file type was found");
  return FileType::Error;
}

QString
  Parser::findXMLElement(QXmlStreamReader &xml,
                         const QString &elemName,
                         const QString &stopEndElem,
                         bool okIfDidntFind,
                         bool stopStartElem)////todo: look for infinite loops
{
  QString result;
  bool reachedStop = false;
  auto stopToken = QXmlStreamReader::TokenType::EndElement;
  if (stopStartElem) stopToken = QXmlStreamReader::TokenType::StartElement;
  while (!xml.atEnd() && !xml.hasError())
  {
    if (xml.tokenType() == stopToken && stopEndElem != "atEnd"
        && xml.name() == stopEndElem)
    {
      reachedStop = true;
      // qDebug()<< "findXMLElem: reached stop elem";
      break;
    }
    if (xml.tokenType() == QXmlStreamReader::TokenType::StartElement
        && xml.name() == elemName)
    {
      if (!xml.attributes().isEmpty())//////todo: consider variable number of
        /// attributes in elements
        xml.readNext();
      result = xml.text().toString();
      break;
    }
    xml.readNext();
  }
  if (xml.hasError()) raiseError(xml.errorString());
  if (!okIfDidntFind && (xml.atEnd() || reachedStop))
    raiseError("xml reader couldn't find the element: " + elemName);
  return result;
}

RaceLogInfo Parser::readXMLLog(bool isRace)
{
  QXmlStreamReader xml(fileData);
  RaceLogInfo result;
  QVector<QString> driverElems;
  if (isRace)
    driverElems = parserConsts::seqElems::DriversRaceElements;
  else
    driverElems = parserConsts::seqElems::DriversPQElements;
  if (!hasError)
  {
    result.SeqElems = processMainLog(xml);
    result.incidents = processIncidents(xml);
    result.drivers = processDrivers(xml, driverElems);
    qDebug() << result;
  }
  return result;
}

QMap<QString, QString> Parser::processMainLog(QXmlStreamReader &xml)
{
  QMap<QString, QString> result;
  const auto seqData = parserConsts::seqElems::MainLogElements;
  const QString XMLEndName = "RaceLaps";
  while (!hasError
         && !(xml.tokenType() == QXmlStreamReader::TokenType::StartElement
              && xml.name() == XMLEndName))
  {
    for (const auto &elem : seqData)/// check if got all elems
    {
      if (hasError) break;
      findXMLElement(xml, elem, XMLEndName);
      qDebug() << xml.name();
      if (xml.name() == XMLEndName) break;
      xml.readNext();
      const auto currentElem = xml.text().toString();
      if (currentElem.isEmpty())
      {
        raiseError("readLog: elem values is empty");
        return QMap<QString, QString>{};/////maybe dont return empty
      }
      result.insert(elem, currentElem);
    }
  }
  return result;
}

bool Parser::openFile(QFile &file, const QIODevice::OpenMode &mode)
{
  if (file.exists())
  {
    if (!file.isOpen())
    {
      if (file.open(mode | QIODevice::Text)) return true;
      raiseError("Can't open file");
    }
    qDebug() << "Is already open!\n";
    return true;
  }
  raiseError("File doesn't exist");
  return false;
}

BackupData Parser::backupFile(const QString &filePath,
                              const QString &backupPath) noexcept
{
  QRegExp dateFilter(R"(\d+_\d{2}_\d{2}_\d{2})");
  QFile file(filePath);
  // set backup file new name
  // get formatted current date and time
  QString backupName{ QDateTime::currentDateTime().date().toString() + '_'
                      + QDateTime::currentDateTime().time().toString() };
  for (auto it = backupName.begin(); it < backupName.end(); ++it)
  {
    if (*it == ':') *it = '_';
  }
  backupName.remove(" ");
  auto datePos = dateFilter.indexIn(backupName);
  backupName = '_' + backupName.remove(0, datePos);
  // get file name format
  auto fileinfo = file.fileName().split(".", QString::KeepEmptyParts);
  const QString fileExt = fileinfo.at(fileinfo.size() - 1);
  fileinfo = file.fileName().split("/", QString::KeepEmptyParts);
  fileinfo =
    fileinfo.at(fileinfo.size() - 1).split(".", QString::KeepEmptyParts);
  const QString fileName = fileinfo.at(0);
  // set new name
  auto newFileName = backupPath + fileName + backupName + "." + fileExt;
  QFile bFile(newFileName);
  // open and write files
  if (file.open(QIODevice::ReadOnly) && bFile.open(QIODevice::WriteOnly)
      && bFile.write(file.readAll()) != -1)
  {
    qDebug() << "backUp: File " << file.fileName()
             << " backed up succesfully. New file is" << bFile.fileName()
             << "\n";
    return BackupData(true, newFileName);
  }
  if (file.isOpen()) file.close();
  if (bFile.isOpen()) bFile.close();
  if (bFile.exists()) bFile.remove();
  qDebug() << "backUp: couldn't open or write file\n";
  return BackupData(false, QString{});
}

bool Parser::restoreFile(const QString &filePath,
                         const QString &backupPath) noexcept
{
  QFile file(filePath);
  QFile bFile(backupPath);
  if (!file.exists() || !bFile.exists())
  {
    qDebug() << "Restore backup error: nowhere or nothing to restore\n";
    return false;
  }
  file.resize(0);
  if (bFile.open(QIODevice::ReadOnly) && file.open(QIODevice::WriteOnly)
      && file.write(bFile.readAll()) != -1)
  {
    qDebug() << "restor Backup err: File " << file.fileName()
             << " restored succesfully\n";
    // delete backup
    if (bFile.exists()) bFile.remove();
    return true;
  }
  if (file.isOpen()) file.close();
  if (bFile.isOpen()) bFile.close();
  qDebug() << "restore backup err: could open or write the files\n";
  return false;
}

bool Parser::readFileContent()/////////TODO: finish, return all data
{
  /// after reading call static methods for writing to db
  if (hasError)
  {
    qDebug() << "readFile: " << errorMessage << '\n';
    return false;
  }
  switch (fileType)
  {
  case FileType::Error: {
    return false;
  }
  case FileType::HDV: {
    break;
  }
  case FileType::RCD: {
    break;
  }
  case FileType::VEH: {
    break;
  }
  case FileType::RaceLog: {
    auto data = readXMLLog(true);
    break;
  }
  case FileType::QualiLog:
  case FileType::PracticeLog: {
    auto data = readXMLLog(false);
    break;
  }
  }
  // catch excepts
  return false;
}

void Parser::raiseError(const QString &msg)
{
  if (!hasError)
  {
    hasError = true;
    if (msg.isEmpty())
    {
      qDebug() << "raiseError: error msg is empty\n";
    } else
    {
      errorMessage = msg;
      qDebug() << "raiseError: " << errorMessage << '\n';/// for debug
    }
  } else
    qDebug() << "there is already an error: " << errorMessage
             << '\n';/// for debug
}

QVector<DriverPair> Parser::processIncidents(QXmlStreamReader &xml)
{
  const QString XMLName = "Incident";
  const QString XMLEndName = "Stream";
  QVector<QString> incidents;
  // parse and save incident strings
  QString prevDriverName;
  while (!hasError
         && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement
              && xml.name() == XMLEndName))
  {
    const auto currentElem = findXMLElement(xml, XMLName, XMLEndName, true);
    if (currentElem.isEmpty())// check if found something, continue if empty
      continue;
    // process two names involved in incident
    auto currentDriverName =
      currentElem.split("(", QString::SkipEmptyParts).first();
    if (prevDriverName != currentDriverName)
    {
      incidents.push_back(currentElem);
      prevDriverName = currentDriverName;
    }
  }
  if (incidents.isEmpty())// check if found something, return empty if not
  {
    qDebug() << "Incidents: no incs were found";
    return QVector<DriverPair>{};
  }
  return processEqualCombinations(incidents);
}

QVector<DriverPair>
  Parser::processEqualCombinations(const QVector<QString> &incindents) const
{
  QVector<DriverPair> incidentData;
  const QString incLineSplitter = "vehicle ";
  // ignore equal driver combinations
  QString currentString;
  const int minValidLineLength = 4;
  for (const auto &i : incindents)////maybe check drivers names validity
  {
    currentString = i;
    auto stringList = currentString.split("(", QString::SkipEmptyParts);
    if (stringList.size() < minValidLineLength) continue;
    const auto firstDriver = stringList.first();
    const auto secondDriver = stringList.at(stringList.size() - 2)
                                .split(incLineSplitter, QString::SkipEmptyParts)
                                .last();
    if (!incidentData.contains(qMakePair(secondDriver, firstDriver)))
      incidentData.push_back(DriverPair(firstDriver, secondDriver));
  }
  return incidentData;
}

QVector<DriverInfo> Parser::processDrivers(QXmlStreamReader &xml,
                                           const QVector<QString> &seqData)
{
  QVector<DriverInfo> drivers;
  const QString XMLName = "Driver";
  const QString XMLEndName = "RaceResults";
  while (!hasError
         && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement
              && xml.name() == XMLEndName))
  {
    findXMLElement(xml, XMLName, XMLEndName, true);
    if (xml.name() != XMLName) break;
    DriverInfo currentDriver;
    for (const auto &elem : seqData)
    {
      if (hasError) break;
      findXMLElement(xml, elem, XMLName);
      if (elem == "Lap")
        currentDriver.lapTimes = processDriverLaps(xml);
      else
        xml.readNext();
      if (xml.text().isEmpty() && elem != "Lap")
      {
        raiseError("processDrivers: elem values is empty");
        return QVector<DriverInfo>{};
      }
      currentDriver.SeqElems.insert(elem, xml.text().toString());
    }
    drivers.push_back(currentDriver);
  }
  return drivers;
}

QVector<QPair<int, double>> Parser::processDriverLaps(QXmlStreamReader &xml)
{
  const QString XMLName = "Lap";
  const QString notTimedLapText = "--.----";
  const QString XMLEndName = "BestLapTime";
  QVector<QPair<int, double>> LapTimes;
  int LapNumber = 1;
  while (!hasError
         && !(xml.tokenType() == QXmlStreamReader::TokenType::StartElement
              && xml.name() == XMLEndName))
  {
    auto laptime = findXMLElement(xml, XMLName, XMLEndName, true, true);
    if (xml.text() != notTimedLapText && !laptime.isEmpty())
    {
      LapTimes.push_back(QPair<int, double>(LapNumber, laptime.toDouble()));
    } else if (!laptime.isEmpty())
    {
      LapTimes.push_back(QPair<int, double>(LapNumber, 0.));
    }
    ++LapNumber;
  }
  return LapTimes;
}
