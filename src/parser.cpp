#include <parser.h>
#include <exception>
#include <QDateTime>
#include <QXmlStreamReader>
//#ifdef QT_DEBUG QT_NO_DEBUG_OUTPUT
//#endif

using namespace parserConsts;

Parser::Parser(QFile &file) :
    fileName(file.fileName())
{
  if (openFile(file, QIODevice::ReadWrite))
  {
    auto buffer = file.readAll();
    if(buffer.isEmpty())
        throw std::runtime_error("Parser init: input file is empty");
    fileData = QString(buffer).toUtf8();
    fileType = readFileType();
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
  try{
    findXMLElement(xml, tokenName);
  }
  catch(std::exception& e)
  {
    throw std::runtime_error(QString("ReadFileType findElem error: ").toStdString() + e.what());
  }
  for (int i = 0; i < 4; ++i)// x4 to get to type token
  {
    xml.readNext();
    if (xml.hasError())
        throw std::runtime_error("fileType read error: " + xml.errorString().toStdString());
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
  throw std::runtime_error("fileType read error: No valid log type was found");
  return FileType::Error;
}

FileType Parser::readModFileType()
{
  QFile file(fileName);
  auto fileinfo = file.fileName().split(".", Qt::KeepEmptyParts);
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
  throw std::runtime_error("fileModType read error: No valid log type was found");
  return FileType::Error;
}

QString Parser::findXMLElement(QXmlStreamReader &xml,
                               const QString &elemName,
                               const QString &stopEndElem,
                               bool okIfDidntFind,
                               bool stopStartElem)
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
      break;
    }
    if (xml.tokenType() == QXmlStreamReader::TokenType::StartElement
        && xml.name() == elemName)
    {
      if (!xml.attributes().isEmpty())
        xml.readNext();
      result = xml.text().toString();
      break;
    }
    xml.readNext();
  }
  if (xml.hasError()) throw std::runtime_error(xml.errorString().toStdString());
  if (!okIfDidntFind && (xml.atEnd() || reachedStop))
    throw std::runtime_error("xml reader couldn't find the element: " + elemName.toStdString());
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
  try {
      result.SeqElems = processMainLog(xml);
      result.incidents = processIncidents(xml);
      result.drivers = processDrivers(xml, driverElems);
      qDebug() << result;
  } catch (std::exception& e) {
      throw std::runtime_error(QString("XMLRead error: ").toStdString() + e.what());
  }
  return result;
}

QMap<QString, QString> Parser::readHDV()
{
  /// look for element seq line by line, if found scan the line
  /// with textstream>> for digits and read the first one
  /// if stream cant get digits it returns 0
  /// if not found elem or val raise error
  QMap<QString, QString> data;
  QTextStream dataStream(&fileData);
  const auto seqData = parserConsts::seqElems::HDVElements;
  auto currElem = seqData.begin();
  while (!dataStream.atEnd() && !(currElem == seqData.end()))
  {
    auto line = dataStream.readLine();
    if (!line.isNull() && line.contains(*currElem))
    {
      double value = 0;
      QTextStream lineScan(&line);
      while (!lineScan.atEnd())
      {
        lineScan.read(1);
        lineScan >> value;
        if (value != 0) break;
      }
      if (value == 0) break;// havent found the value
      QString stringVal;// transform value to string
      QTextStream valStream(&stringVal);
      valStream << value;
      data.insert(*currElem, stringVal);
      ++currElem;
    }
  }
  if (currElem != seqData.end())
    throw std::runtime_error("readHDV: haven't found elemen or its value " + currElem->toStdString());
  return data;
}

QMap<QString, QString> Parser::readVEH()
{
  // look for element seq line by line, if found scan the line
  // read after "=" and remove " if any
  QMap<QString, QString> data;
  QTextStream dataStream(&fileData);
  const auto seqData = parserConsts::seqElems::VEHElements;
  auto currElem = seqData.begin();
  while (!dataStream.atEnd() && !(currElem == seqData.end()))
  {
    auto line = dataStream.readLine();
    if (!line.isNull() && line.contains(*currElem) && !line.contains("//"))
    {
      QString value = line;
      value.remove('=');// remove useless chars
      value.remove(*currElem);
      value.remove('\"');
      data.insert(*currElem, value);
      ++currElem;
    }
  }
  if (currElem != seqData.end())
    throw std::runtime_error("readVEH: haven't found element or its value " + currElem->toStdString());
  return data;
}

QVector<QMap<QString, QString>> Parser::readRCD()
{
  // then cycle read next line and all seq elems within {}
  // do for all the drivers
  auto modString = "Mod";
  auto driverString = "Driver";
  QVector<QMap<QString, QString>> data;
  QTextStream dataStream(&fileData);
  auto line = dataStream.readLine();// read first line which is mod name
  QMap<QString, QString> mod{ { modString, line } };
  data.push_back(mod);
  dataStream.readLine();// set stream into posistion
  const auto seqData = parserConsts::seqElems::RCDElements;
  auto currElem = seqData.begin();
  QString currDriver;
  while (!dataStream.atEnd())
  {
    line = currDriver = dataStream.readLine();// find driver name line
    if (line == '}') break;
    if (line.isNull())
      throw std::runtime_error("readVEH: haven't found elemen or its value " + currDriver.toStdString()
                 + ":" + currElem->toStdString());
    // init driver data map and look for its seq elements
    QMap<QString, QString> DriverData{ { driverString,
                                         currDriver.simplified() } };
    while (!line.contains("}"))
    {
      if (!line.isNull() && line.contains(*currElem))
      {
        // remove all but value
        auto value = line.simplified().remove(0, currElem->size() + 3);
        DriverData.insert(*currElem, value);
        ++currElem;
      }
      line = dataStream.readLine();
      if (dataStream.atEnd()) break;
    }
    if (currElem != seqData.end())
      throw std::runtime_error("readVEH: haven't found elemen or its value " + currDriver.toStdString()
                 + ":" + currElem->toStdString());
    else
    {
      data.push_back(DriverData);
      currElem = seqData.begin();
    }
  }
  return data;
}

QMap<QString, QString> Parser::processMainLog(QXmlStreamReader &xml)
{
  QMap<QString, QString> result;
  const auto seqData = parserConsts::seqElems::MainLogElements;
  const QString XMLEndName = "RaceLaps";
    for (const auto &elem : seqData)/// todo check if got all elems
    {
      findXMLElement(xml, elem, XMLEndName);

      xml.readNext();
      const auto currentElem = xml.text().toString();
      if (currentElem.isEmpty())
        throw std::runtime_error("readLog: elem value is empty");
      result.insert(elem, currentElem);
            if (xml.name() == XMLEndName) break;
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
      throw std::runtime_error("Can't open file");
    }
    qDebug() << "File Is already open!\n";
    return true;
  }
  throw std::runtime_error("File doesn't exist");
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
  auto fileinfo = file.fileName().split(".", Qt::KeepEmptyParts);
  const QString fileExt = fileinfo.at(fileinfo.size() - 1);
  fileinfo = file.fileName().split("/", Qt::KeepEmptyParts);
  fileinfo =
    fileinfo.at(fileinfo.size() - 1).split(".", Qt::KeepEmptyParts);
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
    if (file.isOpen()) file.close();
    if (bFile.isOpen()) bFile.close();
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
  try{
  switch (fileType)
  {
  case FileType::HDV: {
    auto data = readHDV();
    ///////// debug
    auto it = data.cbegin();
    qDebug() << "==========HDV data========\n";
    while (it != data.cend())
    {
      qDebug() << it.key() << " " << it.value() << '\n';
      ++it;
    }
    //////// end debug
    break;
  }
  case FileType::RCD: {
    auto data = readRCD();
    ///////// debug

    qDebug() << "==========RCD data========\n";
    for (const auto &i : data)
    {
      auto it = i.cbegin();
      while (it != i.cend())
      {
        qDebug() << it.key() << " " << it.value() << '\n';
        ++it;
      }
    }
    //////// end debug
    break;
  }
  case FileType::VEH: {
    auto data = readVEH();
    ///////// debug
    auto it = data.cbegin();
    qDebug() << "==========VEH data========\n";
    while (it != data.cend())
    {
      qDebug() << it.key() << " " << it.value() << '\n';
      ++it;
    }
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
  default:
      return false;
  }
}
  catch(std::exception& e)
  {
    qDebug() << "\nreadFile error: " << e.what() << '\n';
    return false;
  }
    /// todo: figure out how to return
    /// after reading call static methods for writing to db
    return true;
}

QVector<DriverPair> Parser::processIncidents(QXmlStreamReader &xml)
{
  const QString XMLName = "Incident";
  const QString XMLEndName = "Stream";
  QVector<QString> incidents;
  // parse and save incident strings
  QString prevDriverName;
  while (!(xml.tokenType() == QXmlStreamReader::TokenType::EndElement
              && xml.name() == XMLEndName))
  {
    const auto currentElem = findXMLElement(xml, XMLName, XMLEndName, true);
    if (currentElem.isEmpty())// check if found something, continue if empty
      continue;
    // process two names involved in incident
    auto currentDriverName =
      currentElem.split("(", Qt::SkipEmptyParts).first();
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
    auto stringList = currentString.split("(", Qt::SkipEmptyParts);
    if (stringList.size() < minValidLineLength) continue;
    const auto firstDriver = stringList.first();
    const auto secondDriver = stringList.at(stringList.size() - 2)
                                .split(incLineSplitter, Qt::SkipEmptyParts)
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
  while (!(xml.tokenType() == QXmlStreamReader::TokenType::EndElement
              && xml.name() == XMLEndName))
  {
    findXMLElement(xml, XMLName, XMLEndName, true);
    if (xml.name() != XMLName) break;
    DriverInfo currentDriver;
    for (const auto &elem : seqData)
    {
      findXMLElement(xml, elem, XMLName);
      if (elem == "Lap")
        currentDriver.lapTimes = processDriverLaps(xml);
      else
        xml.readNext();
      if (xml.text().isEmpty() && elem != "Lap")
        throw std::runtime_error("processDrivers: elem value is empty");
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
  while (!(xml.tokenType() == QXmlStreamReader::TokenType::StartElement
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
