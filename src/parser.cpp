#include <parser.h>
#include <exception>
#include <QDateTime>
#include <QXmlStreamReader>


Parser::Parser(QFile &file) : fileName(file.fileName())
{
  if (openFile(file, QIODevice::ReadWrite))
  {
    auto buffer = file.readAll();
    if (buffer.isEmpty())
      throw std::runtime_error("Parser init: input file is empty");
    setFileData(QString(buffer).toUtf8());
  }
}

Parser::~Parser()
{
  QFile file(fileName);
  if (file.isOpen())
    file.close();
}

BackupData Parser::backupFile(const QString &filePath, const QString &backupPath) noexcept
{
  QRegExp dateFilter(R"(\d+_\d{2}_\d{2}_\d{2})");
  QFile file(filePath);
  // set backup file new name
  // get formatted current date and time
  QString backupName{ QDateTime::currentDateTime().date().toString() + '_'
                      + QDateTime::currentDateTime().time().toString() };
  for (auto it = backupName.begin(); it < backupName.end(); ++it)
  {
    if (*it == ':')
      *it = '_';
  }
  backupName.remove(" ");
  auto datePos = dateFilter.indexIn(backupName);
  backupName = '_' + backupName.remove(0, datePos);
  // get file name format
  auto fileinfo = file.fileName().split(".", Qt::KeepEmptyParts);
  const QString fileExt = fileinfo.at(fileinfo.size() - 1);
  fileinfo = file.fileName().split("/", Qt::KeepEmptyParts);
  fileinfo = fileinfo.at(fileinfo.size() - 1).split(".", Qt::KeepEmptyParts);
  const QString fileName = fileinfo.at(0);
  // set new name
  auto newFileName = backupPath + fileName + backupName + "." + fileExt;
  QFile bFile(newFileName);
  // open and write files
  if (file.open(QIODevice::ReadOnly) && bFile.open(QIODevice::WriteOnly)
      && bFile.write(file.readAll()) != -1)
  {
    qDebug() << "backUp: File " << file.fileName()
             << " backed up succesfully. New file is" << bFile.fileName() << "\n";
    if (file.isOpen())
      file.close();
    if (bFile.isOpen())
      bFile.close();
    return BackupData{ true, newFileName };
  }
  if (file.isOpen())
    file.close();
  if (bFile.isOpen())
    bFile.close();
  if (bFile.exists())
    bFile.remove();
  qDebug() << "backUp: couldn't open or write file\n";
  return BackupData{ false, QString{} };
}

bool Parser::restoreFile(const QString &filePath, const QString &backupPath) noexcept
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
    qDebug() << "restore Backup: File " << file.fileName() << " restored succesfully\n";
    // delete backup
    if (bFile.exists())
      bFile.remove();
    return true;
  }
  if (file.isOpen())
    file.close();
  if (bFile.isOpen())
    bFile.close();
  qDebug() << "restore backup err: could open or write the file\n";
  return false;
}

bool Parser::openFile(QFile &file, const QIODevice::OpenMode &mode)
{
  if (file.exists())
  {
    if (!file.isOpen())
    {
      if (file.open(mode | QIODevice::Text))
        return true;
      throw std::runtime_error("Can't open file");
    }
    qDebug() << "File Is already open!\n";
    return true;
  }
  throw std::runtime_error("File doesn't exist");
}


QVariant XmlParser::readFileContent()
{
  try
  {
    auto data = readXMLLog(parserConsts::seqElems::DriversPQElements);
    //QString dataS;
    //dataS << data;
    //qDebug() << dataS;/// debug
    return QVariant::fromValue(data);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("readFile error: ") + e.what());
  }
}

FileType XmlParser::readFileType()
{
  // find log type token
  QXmlStreamReader xml(fileData);
  try
  {
    const auto tokenName = "FixedUpgrades";  // elem prior to one needed
    findXMLElement(xml, tokenName);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("ReadFileType findElem error: ") + e.what());
  }
  for (int i = 0; i < 4; ++i)  // x4 to get to type token
  {
    xml.readNext();
    if (xml.hasError())
      throw std::runtime_error("fileType read error: " + xml.errorString().toStdString());
  }
  if (xml.name() == getFileTypeById(FileType::RaceLog))
    return FileType::RaceLog;
  if (xml.name() == getFileTypeById(FileType::QualiLog))
    return FileType::QualiLog;
  if (xml.name().contains(getFileTypeById(FileType::PracticeLog)))
    return FileType::PracticeLog;
  throw std::runtime_error("fileType read error: No valid log type was found");
}

QString XmlParser::findXMLElement(QXmlStreamReader &xml,
                                  const QString &elemName,
                                  const QString &stopEndElem,
                                  bool okIfDidntFind,
                                  bool stopStartElem)
{
  QString result;
  bool reachedStop = false;
  auto stopToken = QXmlStreamReader::TokenType::EndElement;
  if (stopStartElem)
    stopToken = QXmlStreamReader::TokenType::StartElement;
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
    /// todo temp solution for bLap parsing
    if (xml.name() == "BestLapTime" && elemName == "Lap")
    {
      xml.readNext();
      result = xml.text().toString();
      findXMLElement(xml, "Laps");
      break;
    }
    xml.readNext();
  }
  if (xml.hasError())
    throw std::runtime_error(xml.errorString().toStdString());
  if (!okIfDidntFind && (xml.atEnd() || reachedStop))
    throw std::runtime_error("xml reader couldn't find the element: "
                             + elemName.toStdString());
  return result;
}

RaceLogInfo XmlParser::readXMLLog(const QVector<QString> &ListOfElems)
{
  QXmlStreamReader xml(fileData);
  RaceLogInfo result;
  try
  {
    result.SeqElems = processMainLog(xml);
    result.incidents = processIncidents(xml, true);
    result.drivers = processDrivers(xml, ListOfElems);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("XMLRead error: ") + e.what());
  }
  return result;
}

QVector<QPair<QString, QString>> XmlParser::processMainLog(QXmlStreamReader &xml)
{
  QVector<QPair<QString, QString>> result;
  const auto seqData = parserConsts::seqElems::MainLogElements;
  const QString XMLEndName = "RaceLaps";
  for (const auto &elem : seqData)
  {
    findXMLElement(xml, elem, XMLEndName);
    xml.readNext();
    const auto currentElem = xml.text().toString();
    if (currentElem.isEmpty())
      throw std::runtime_error("readLog: elem value is empty");
    result.push_back({ elem, currentElem });
    if (xml.name() == XMLEndName)
      break;
  }
  return result;
}

QVector<StringPair> XmlParser::processIncidents(QXmlStreamReader &xml, bool driversOnly)
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
    if (currentElem.isNull())  // check if found something, continue if empty
      continue;
    // process two names involved in incident
    auto currentDriverName = currentElem.split("(", Qt::SkipEmptyParts).first();
    if (prevDriverName != currentDriverName)
    {
      incidents.push_back(currentElem);
      prevDriverName = currentDriverName;
    }
  }
  if (incidents.isEmpty())  // check if found something, return empty if not
    return QVector<StringPair>{};
  if (driversOnly)
    return processEqualCombinations(incidents);
  else
    return parseIncDriverName(incidents);
}

QVector<StringPair>
  XmlParser::processEqualCombinations(const QVector<QString> &incindents) const
{
  QVector<StringPair> incidentData;
  const QString incLineSplitter = "vehicle ";
  // ignore equal driver combinations
  QString currentString;
  const int minValidLineLength = 4;
  for (const auto &i : incindents)  ////maybe check drivers names validity
  {
    currentString = i;
    auto stringList = currentString.split("(", Qt::SkipEmptyParts);
    if (stringList.size() < minValidLineLength)
      continue;
    const auto firstDriver = stringList.first();
    const auto secondDriver = stringList.at(stringList.size() - 2)
                                .split(incLineSplitter, Qt::SkipEmptyParts)
                                .last();
    if (!incidentData.contains(qMakePair(secondDriver, firstDriver)))
      incidentData.push_back(StringPair(firstDriver, secondDriver));
  }
  return incidentData;
}

QVector<DriverInfo> XmlParser::processDrivers(QXmlStreamReader &xml,
                                              const QVector<QString> &seqData)
{
  QVector<DriverInfo> drivers;
  const QString XMLName = "Driver";
  const QString XMLEndName = "RaceResults";
  while (!(xml.tokenType() == QXmlStreamReader::TokenType::EndElement
           && xml.name() == XMLEndName))
  {
    findXMLElement(xml, XMLName, XMLEndName, true);
    if (xml.name() != XMLName)
      break;
    DriverInfo currentDriver;
    for (const auto &elem : seqData)
    {
      if (elem == "Lap")
      {
        const auto laps = processDriverLaps(xml);
        currentDriver.lapTimes = laps.second;
        const auto lapData = generateLapData(currentDriver.lapTimes);
        currentDriver.SeqElems.push_back({ elem, lapData });
        //look for best lap but there may not be one
        if (laps.first.isEmpty())
          currentDriver.SeqElems.push_back({ elem, QString::number(0.0) });
        else
          currentDriver.SeqElems.push_back({ "BestLapTime", laps.first });
        continue;
      }
      if (elem == "BestLapTime")
        continue;
      findXMLElement(xml, elem, XMLName);
      xml.readNext();
      if (xml.text().isEmpty())
        throw std::runtime_error("processDrivers: elem value is empty");
      currentDriver.SeqElems.push_back({ elem, xml.text().toString() });
    }
    drivers.push_back(currentDriver);
  }
  return drivers;
}

QPair<QString, QVector<QPair<int, double>>>
  XmlParser::processDriverLaps(QXmlStreamReader &xml)
{  // there may not be any lap data
  const QString XMLName = "Lap";
  const QString notTimedLapText = "--.----";
  const QString XMLEndName = "Laps";
  QVector<QPair<int, double>> LapTimes;
  int LapNumber = 1;
  QString bLapTime;
  bool noBLap = false;
  // look fo bestlap at the end, if last elem if empty then no best lap
  while (!(xml.tokenType() == QXmlStreamReader::TokenType::StartElement
           && xml.name() == XMLEndName))
  {
    auto laptime = findXMLElement(xml, XMLName, XMLEndName, true, true);
    if (xml.text() != notTimedLapText && !laptime.isNull())
    {
      LapTimes.push_back(QPair<int, double>(LapNumber, laptime.toDouble()));
    } else if (!laptime.isNull())
    {
      LapTimes.push_back(QPair<int, double>(LapNumber, 0.));
    }
    if (laptime.isNull())
      noBLap = true;
    ++LapNumber;
  }
  if (!noBLap)
  {
    bLapTime = QString::number(LapTimes.last().second);
    LapTimes.pop_back();
  }
  return { bLapTime, LapTimes };
}

QString XmlParser::generateLapData(QVector<QPair<int, double>> data)
{
  QString lapString;
  if (data.isEmpty())
    return lapString;
  QTextStream dataS(&lapString);
  for (const auto &i : data) dataS << i.second << ", ";
  lapString.remove(lapString.size() - 2, 2);
  return lapString;
}


QVector<StringPair>
  XmlParser::parseIncDriverName(const QVector<QString> &incindents) const
{
  QVector<StringPair> incidentData;
  QString currentString;
  const int minValidLineLength = 3;
  for (const auto &i : incindents)  ////maybe check drivers names validity
  {
    currentString = i;
    auto stringList = currentString.split("(", Qt::SkipEmptyParts);
    if (stringList.size() < minValidLineLength)
      continue;
    const auto Driver = stringList.first();
    incidentData.push_back(StringPair(Driver, QString()));
  }
  return incidentData;
}

QVariant RXmlParser::readFileContent()
{
  try
  {
    auto data = readXMLLog(parserConsts::seqElems::DriversRaceElements);
    return QVariant::fromValue(data);
  } catch (std::exception &e)
  {
    throw std::runtime_error(QString("readFile error: ").toStdString() + e.what());
  }
}

FileType ModParser::readFileType()
{
  QFile file(fileName);
  auto fileinfo = file.fileName().split(".", Qt::KeepEmptyParts);
  const QString &fileExt = fileinfo.at(fileinfo.size() - 1);
  if (fileExt == getFileTypeById(FileType::RCD))
  {
    qDebug() << "rcd\n";
    return FileType::RCD;
  }
  if (fileExt == getFileTypeById(FileType::HDV))
  {
    qDebug() << "hdv\n";
    return FileType::HDV;
  }
  if (fileExt == getFileTypeById(FileType::VEH))
  {
    qDebug() << "veh\n";
    return FileType::VEH;
  }
  if (fileExt == getFileTypeById(FileType::INI))
  {
    qDebug() << "ini\n";
    return FileType::INI;
  }
  throw std::runtime_error("fileModType read error: No valid log type was found");
}

void ModParser::delJunkInLine(QString &line, bool delWhiteSpace)
{
  int commentPos = line.indexOf("//");
  if (commentPos != -1)
    line = line.left(commentPos);
  if (delWhiteSpace)
    line = line.simplified().remove(' ');
}


bool RCDParser::writeChanges()
{
  QString newData;
  const QString prefix(QString("%1%2%3").arg('\n').arg(INIFile::tokenOpen).arg('\n'));
  const QString postfix(QString("%1%2%3").arg('\n').arg(INIFile::tokenClose).arg("\n\n"));
  newData.append(vehClassName + prefix);
  auto driverIt = data.begin();
  while (driverIt != data.end())
  {
    newData.append(driverIt.key() + prefix);
    auto rcdIt = driverIt->begin();
    while (rcdIt != driverIt->end())
    {
      newData.append(rcdIt->name + " = " + QString::number(rcdIt->value) + '\n');
      rcdIt++;
    }
    newData.append(postfix);
    driverIt++;
  }
  newData.append(postfix);
  return doWriteModFile(newData);
}

void RCDParser::addDriver(const QString &name)
{
  data.insert(name, seqElems::RCDElements);
}

void RCDParser::modifyEntry(const QString &name, RCDEntries entry, double newVal)
{
  if (data.contains(name))
    data[name][entry].value = newVal;
  else
    throw std::runtime_error("RCDParser::modifyEntry: no element named "
                             + name.toStdString());
}

void RCDParser::removeEntry(const QString &name) { data.remove(name); }


QVariant RCDParser::readFileContent()
{
  try
  {
    DriverStats result;
    QTextStream dataStream(&fileData);
    // look for veh class name, could be comments beforehand
    QString lastTokenName;
    while (!dataStream.atEnd())
    {
      const auto line = dataStream.readLine();
      if (line.contains(INIFile::tokenOpen))
        break;

      lastTokenName = line;
      ModParser::delJunkInLine(lastTokenName, true);
    }
    assert(!dataStream.atEnd());
    vehClassName = lastTokenName;
    // then cycle read next line and all seq elems within {}
    // do for all the drivers in a file
    // skip "default" entry
    while (!dataStream.atEnd())
    {
      auto line = dataStream.readLine();  // find driver name line
      if (line.contains(INIFile::tokenOpen) && !lastTokenName.contains(ignoreEntry))
      {
        auto seqData = parserConsts::seqElems::RCDElements;
        while (!dataStream.atEnd() && !line.contains(INIFile::tokenClose))
        {
          line = dataStream.readLine();
          auto elemIt =
            std::find_if(seqData.begin(), seqData.end(), [&line](const auto &i) {
              return line.contains(i.name);
            });
          if (elemIt != seqData.end())
          {
            bool converted;
            elemIt->value =
              HDVParser::getElemValueStr(elemIt->name, line).toDouble(&converted);
            if (!converted)
              throw std::runtime_error(
                std::string("readRCD: invalid line: " + line.toStdString()));
          }
        }
        result.insert(lastTokenName, seqData);
      }
      lastTokenName = line;
      ModParser::delJunkInLine(lastTokenName, false);
    }
    return QVariant::fromValue(result);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("readRCD: readFile error: ") + e.what());
  }
}


void VEHParser::modifyDriverName(const QString &newVal)
{
  currentData[seqElems::VEHElemIndexes::Driver].second = newVal;
}

bool VEHParser::writeChanges()
{
  QVector<WriteDataInput<QString>> writeData(currentData.size());
  for (int i = 0; i < writeData.size(); i++)
  {
    writeData[i].elemName = originalData[i].first;
    switch (i) {
      case seqElems::VEHElemIndexes::Driver:
      case seqElems::VEHElemIndexes::Classes:
        writeData[i].oldVal = '\"' + originalData[i].second + '\"';
        writeData[i].newVal = '\"' + currentData[i].second + '\"';
        break;
      default:
        writeData[i].oldVal = originalData[i].second;
        writeData[i].newVal = currentData[i].second;
        break;
    }
  }
  return updateModFileData(writeData);
}

QVariant VEHParser::readFileContent()
{
  try
  {
    // look for element seq line by line, if found scan the line
    // read value after "=" and remove " if any
    QVector<StringPair> result;
    QTextStream dataStream(&fileData);
    auto seqData = parserConsts::seqElems::VEHElements;
    while (!dataStream.atEnd())
    {
      auto line = dataStream.readLine();
      if (!line.isNull())
      {
        delJunkInLine(line, false);
        auto elemIt = std::find_if(seqData.begin(),
                                   seqData.end(),
                                   [&line](const auto &i) { return line.contains(i); });

        if (elemIt != seqData.end())
        {
          int pos = line.indexOf(INIFile::tokenValBegin);
          if (pos == -1)
            std::runtime_error("readVEH: haven't found value for element: "
                               + elemIt->toStdString());
          if (line.at(pos + 1) == '\"')
          {
            pos += 2;  // skip '="'
            int endPos = line.indexOf('\"', pos);  // find closing "
            if (endPos == -1)
              std::runtime_error("readVEH: haven't found value for element: "
                                 + elemIt->toStdString());
            line = line.mid(pos, endPos - pos);
          } else
          {
            line = line.mid(pos + 1);
            line.remove(' ');
          }
          result.push_back({ *elemIt, line });
          seqData.erase(elemIt);
        }
      }
    }
    if (!seqData.isEmpty())
      throw std::runtime_error("readVEH: haven't found element or its value");
    return QVariant::fromValue(result);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("readFile error: ") + e.what());
  }
}


QVariant HDVParser::readFileContent()
{
  try
  {
    /// look for element seq line by line, if found scan the line
    /// with textstream>> for digits and read the first one
    /// if stream cant get digits it returns 0
    /// if not found elem or val raise error
    HDVParams result;
    QTextStream dataStream(&fileData);
    const auto seqData =
      missingParams.empty() ? parserConsts::seqElems::HDVElements : missingParams;
    auto currElem = seqData.begin();
    while (!dataStream.atEnd() && !(currElem == seqData.end()))
    {
      auto line = dataStream.readLine();
      if (!line.isNull() && line.contains(currElem->name))
      {
        result.insert(currElem.key(),
                      HDVData{ currElem->prefix,
                               currElem->name,
                               getElemValueStr(currElem->name, line) });
        ++currElem;
      }
    }
    if (currElem != seqData.end())
      throw std::runtime_error("readHDV: haven't found elemen or its value "
                               + currElem->name.toStdString());
    return QVariant::fromValue(result);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("readFile error: ") + e.what());
  }
}

QString HDVParser::getElemValueStr(const QString &elemName, const QString &line)
{
  const std::string err("HDV Read error: invalid element in getElemValueStr");
  int pos = line.indexOf(elemName);
  QString value;
  if (pos == -1)
    throw std::runtime_error(err);
  else
  {
    pos = line.indexOf(INIFile::tokenValBegin, pos);
    if (pos == -1)
      throw std::runtime_error(err);
    value = line.mid(pos + 1);
    ModParser::delJunkInLine(value, true);
  }
  return value;
}


QVariant INIParser::readFileContent()
{
  HDVParams result;
  HDVParams elems = parserConsts::seqElems::HDVElements;
  bool foundUpgradeToken = false;
  long long lastTokenPos = -1;
  QTextStream dataStream(&fileData);
  QString line;
  while (!dataStream.atEnd() && !(elems.empty()))
  {
    long long linePos = dataStream.pos();
    line = dataStream.readLine();
    if (line.contains(INIFile::tokenUpgradeType))
      lastTokenPos = linePos;
    auto elemIt = std::find_if(elems.begin(), elems.end(), [&line](const auto &i) {
      return line.contains(i.name);
    });
    if (elemIt != elems.end())
    {
      result.insert(elemIt.key(),
                    HDVData{ elemIt->prefix,
                             elemIt->name,
                             HDVParser::getElemValueStr(elemIt->name, line) });
      elems.erase(elemIt);
      foundUpgradeToken = true;
      break;
    }
  }
  if (foundUpgradeToken)
  {  // find available elems values
    while (!dataStream.atEnd() && !line.contains(INIFile::tokenClose))
    {
      line = dataStream.readLine();
      auto elemIt = std::find_if(elems.begin(), elems.end(), [&line](const auto &i) {
        return line.contains(i.name);
      });
      if (elemIt != elems.end())
      {
        result.insert(elemIt.key(),
                      HDVData{ elemIt->prefix,
                               elemIt->name,
                               HDVParser::getElemValueStr(elemIt->name, line) });
        elems.erase(elemIt);
      }
    }
    line = dataStream.readLine();
    // get end pos of current token
    if (!dataStream.atEnd() && !line.contains(INIFile::tokenClose))
    {  // found another sub token
      while (!dataStream.atEnd() && !line.contains(INIFile::tokenClose))
        line = dataStream.readLine();
    }
    missingData = elems;
    info.upgradeTokenPos = lastTokenPos;
    info.upgradeTokenSize = dataStream.pos() - lastTokenPos + 1;
  }
  return QVariant::fromValue(result);
}

bool INIParser::writeChanges()
{
  using namespace INIFile;  //!

  auto newData = fileData;
  if (info.initialized())
  {  // remove old token
    newData.remove(int(info.upgradeTokenPos), int(info.upgradeTokenSize));
  }
  // replace with current values
  const QString tokenPrefix = QString("\n%1=%2\n%3\n%4=%5\n%6\n")
                                .arg(tokenUpgradeType)
                                .arg(tokenUpgradeTypeName)
                                .arg(tokenOpen)
                                .arg(tokenUpgradeLevel)
                                .arg(tokenUpgradeLevelName)
                                .arg(tokenOpen);
  newData.append(tokenPrefix);
  QString lastPrefix;
  for (const auto &e : data)
  {
    if (e.prefix != lastPrefix)
    {
      newData.append(elemPrefix + '\n');
      newData.append(elemPrefix + e.prefix + '\n');
    }
    newData.append(elemPrefix + e.name + '=' + e.value + '\n');
    lastPrefix = e.prefix;
  }
  newData.append(QString("%1\n%2\n%3\n").arg(elemPrefix).arg(tokenClose).arg(tokenClose));

  return doWriteModFile(newData);
}

void INIParser::modifyEntry(HDVEntries entry, const QString &newVal)
{
  if (data.contains(entry))  // if no entry present insert as is (from HDVParser)
    data[entry].value = getStrValueForEntry(entry, newVal, data[entry].value);
  else
    data.insert(
      entry,
      { seqElems::HDVElements[entry].prefix, seqElems::HDVElements[entry].name, newVal });
}

double INIParser::getValueFromStr(const QString &str)
{
  /* possible value strings:
   * (-0.288,-0.0141,0.000064)
   * (0.35475)
   * 1.0
   * 10
   */
  QString tempStr = str;
  auto sections = tempStr.split(',');
  tempStr = sections.at(0);
  tempStr.remove(QRegularExpression("[(|)]+"));
  bool converted;
  double result = tempStr.toDouble(&converted);
  if (!converted)
    throw std::runtime_error("INIParser getValueFromStr error: unable to convert value");
  return result;
}

QString INIParser::getStrValueForEntry(HDVEntries entry,
                                       const QString &newValue,
                                       const QString &oldValStr)
{
  bool converted = false;
  QString result;
  switch (entry)
  {
  case HDVEntries::FWSetting:
  case HDVEntries::RWSetting:
    result = QString::number(newValue.toInt(&converted));
    break;
  case HDVEntries::BodyDragBase:
    result = '(' + QString::number(newValue.toDouble(&converted)) + ')';
    break;
  case HDVEntries::GeneralPowerMult:
    result = QString::number(newValue.toDouble(&converted));
    break;
  default:  // braced, coma separated values
  {
    int pos = oldValStr.indexOf(',');
    assert(pos != -1);
    QString remainder = oldValStr.mid(pos);
    result = '(' + QString::number(newValue.toDouble(&converted)) + remainder;
  }
  }
  if (!converted)
    throw std::runtime_error(
      "INIParser getStrValueForEntry error: unable to convert value");
  return result;
}


bool ModParser::doWriteModFile(const QString &data)
{
  // backup before write, then open and try to write, if failed try to restore
  /// backup path is the same as original
  auto pathWOname = fileName;
  pathWOname = pathWOname.remove(fileName.split('/').last());
  const BackupData backUpRes{ backupFile(fileName, pathWOname) };
  if (!backUpRes.result)
  {
    qDebug() << "write mod file: couldnt back up the file";
    return false;
  }
  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly) && file.write(data.toStdString().c_str()) != -1)
  {
    setFileData(data);
    qDebug() << "write mod file: success!";
    QFile bFile(backUpRes.fileFullPath);  // delete backup
    if (bFile.exists())
      bFile.remove();
    return true;
  } else
  {
    // try to restore from backup
    restoreFile(fileName, backUpRes.fileFullPath);
    return false;
  }
}


RaceLogInfo DataSetParser::readXMLLog(const QVector<QString> &ListOfElems)
{
  QXmlStreamReader xml(fileData);
  RaceLogInfo result;
  try
  {
    //const QString XMLName = "RaceLaps";
    //findXMLElement(xml, XMLName, XMLName);
    result.incidents = processIncidents(xml, false);
    result.drivers = processDrivers(xml, ListOfElems);
  } catch (std::exception &e)
  {
    throw std::runtime_error(std::string("XMLRead error: ") + e.what());
  }
  return result;
}

/// dataset format
/// +-------------+-----------+------+----------+---------+-----+
/// | SessionType | Incidents | Name | TeamName | GridPos | Pos |
/// +-------------+-----------+------+----------+---------+-----+
/// | str         | Y/N       | str  | str      | INT     | INT |
/// +-------------+-----------+------+----------+---------+-----+
QVector<QString>
  DataSetParser::preprocessDataSet(const QPair<QString, RaceLogInfo> &dataSet,
                                   bool displayHeaders)
{
  if (dataSet.second.drivers.isEmpty())
    throw std::runtime_error("preprocessData: no driver data");
  const QString sessionType = "SessionType";
  const QString incidents = "Incidents";
  const char incY = 'Y';
  const char incN = 'N';
  QVector<QString> result;
  bool noIncidents = false;
  const bool isRace = dataSet.first == getFileTypeById(FileType::RaceLog);
  if (dataSet.second.incidents.isEmpty())
    noIncidents = true;
  QVector<QString> headers{ sessionType, incidents };
  for (const auto &i : seqElems::DataSetRElems) headers.push_back(i);
  // check data table dimensions
  const int headerSize = headers.size();
  if (dataSet.second.drivers.at(0).SeqElems.size() == headerSize)
    throw std::runtime_error("preprocessData: wrong data dimensions");
  if (displayHeaders)
    result.push_back(makeSCVLine(headers));  // added headers row

  for (const auto &driver : dataSet.second.drivers)
  {
    QVector<QString> line{ dataSet.first };
    if (noIncidents)
      line.push_back(QString{ incN });
    else
    {
      const auto name = driver.SeqElems.at(0).second;  // driver name element
      if (hasIncidents(name, dataSet.second.incidents))
        line.push_back(QString{ incY });
      else
        line.push_back(QString{ incN });
    }  // added session type and incidents values to the row
    if (isRace)
      for (const auto &i : driver.SeqElems) line.push_back(i.second);
    else
    {
      for (const auto &i : driver.SeqElems)
      {
        if (i.first == "Position")
        {
          line.push_back("0");  /// todo temp
          line.push_back(i.second);
        } else
          line.push_back(i.second);
      }
    }
    result.push_back(makeSCVLine(line));
  }
  return result;
}

QString DataSetParser::makeSCVLine(const QVector<QString> &data)
{
  if (data.isEmpty())
    throw std::runtime_error("makeSCVLine: empty line");
  bool firsVal = true;
  QString result;
  for (const auto &i : data)
  {
    if (firsVal)
    {
      result.push_back(i);
      firsVal = false;
    } else
      result.push_back(',' + i);
  }
  return result;
}

bool DataSetParser::hasIncidents(const QString &driver,
                                 const QVector<StringPair> &incidents)
{
  const auto result =
    std::find_if(incidents.begin(), incidents.end(), [&driver](const auto &i) {
      return i.first == driver;
    });
  return result != incidents.end();
}

bool DataSetParser::convertToCSV(const QVector<QPair<QString, RaceLogInfo>> &dataSet,
                                 QFile &oFile)
{
  if (!oFile.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;
  oFile.resize(0);
  QTextStream stream(&oFile);
  bool first = true;
  for (const auto &logInfo : dataSet)
  {
    QVector<QString> rawStrings;
    if (first)
    {
      rawStrings = preprocessDataSet(logInfo, true);
      first = false;
    } else
      rawStrings = preprocessDataSet(logInfo, false);
    if (!rawStrings.isEmpty())
      for (const auto &line : rawStrings) stream << line << Qt::endl;
    else
    {
      if (oFile.exists())
      {
        oFile.close();
        oFile.remove();
      }
      return false;
    }
  }
  if (oFile.isOpen())
    oFile.close();
  return true;
}
