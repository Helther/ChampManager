#ifndef parser_H
#define parser_H
#include <parserConsts.h>

using DriverPair = QPair<QString, QString>;

struct BackupData
{
  BackupData(bool inRes, const QString &inFilePath)
    : result(inRes), fileFullPath(inFilePath)
  {}
  bool result = false;
  QString fileFullPath;
};

struct DriverInfo
{
  QMap<QString, QString> SeqElems;
  QVector<QPair<int, double>> lapTimes;
};

#ifdef QT_DEBUG
inline QDebug operator<<(QDebug debug, const DriverInfo &dr)
{
  auto it = dr.SeqElems.cbegin();
  QDebugStateSaver saver(debug);
  debug.nospace() << "==========Driver data========\n";
  while (it != dr.SeqElems.cend())
  {
    debug << it.key() << " " << it.value() << '\n';
    ++it;
  }
  debug.nospace() << "===========Laps==============\n";
  for (const auto &i : dr.lapTimes)
    debug.nospace() << "# " << i.first << " " << i.second << '\n';
  return debug;
}
#endif

struct RaceLogInfo
{
  QMap<QString, QString> SeqElems;
  QVector<DriverInfo> drivers;
  QVector<DriverPair> incidents;
};

#ifdef QT_DEBUG
inline QDebug operator<<(QDebug debug, const RaceLogInfo &log)
{
  auto it = log.SeqElems.cbegin();
  QDebugStateSaver saver(debug);
  debug.nospace() << "==========xml log data========\n";
  debug.nospace() << "==========Incidents============\n";
  for (const auto &i : log.incidents)
    debug.nospace() << "Incident between " << i.first << " and " << i.second
                    << '\n';
  debug.nospace() << "==========Incidents END============\n";
  while (it != log.SeqElems.cend())
  {
    debug.nospace() << it.key() << " " << it.value() << '\n';
    ++it;
  }
  for (const auto &i : log.drivers) debug << i;
  return debug;
}
#endif

struct WriteData
{
  QTextStream data;
  QString line;
  int index;
};

template<typename T> struct WriteDataInput
{
  QString elemName;
  T oldVal;
  T newVal;
};

class Parser
{
public:
  explicit Parser(QFile &file);
  ~Parser();

  // straightforward name, gives backup a name with a current date
  // returns bool result and full backup file path
  // deletes backup and empy file path if not successful
  static BackupData backupFile(const QString &filePath,
                               const QString &backupPath) noexcept;
  static bool restoreFile(const QString &filePath,
                          const QString &backupPath) noexcept;

  // read file for all info, depending on filetype
  bool readFileContent();
  // looks for values into specific element names given names/values list
  template<typename T>
  bool updateModFileData(const QVector<WriteDataInput<T>> &input);

  //=============================getters===========================/
  [[nodiscard]] constexpr FileType getFileType() const { return fileType; }
  [[nodiscard]] const QString getFileData() const { return fileData; }
  [[nodiscard]] const QString getFileName() const { return fileName; }

  void setFileData(const QString &inData) { fileData = inData; }

private:
  // just opens file with error catching
  [[nodiscard]] bool openFile(QFile &file, const QIODevice::OpenMode &mode);

  // finds type of file
  [[nodiscard]] FileType readFileType();
  [[nodiscard]] FileType readModFileType();
  [[nodiscard]] constexpr bool isModFile()
  {
    return ((fileType == FileType::HDV) || (fileType == FileType::RCD)
            || (fileType == FileType::VEH));
  }

  // reads through file using xml reader reference in search of given element
  // name, returns element value, if never found returns empty qstring
  QString findXMLElement(QXmlStreamReader &xml,
                         const QString &elemName,
                         const QString &stopEndElem = "atEnd",
                         bool okIfDidntFind = false,
                         bool stopStartElem = false);

  // writes resulting changes to a file
  bool doWriteModFile(const QString &data);
  // looks for element with a given name and value, returns true if successful
  template<typename T>
  [[nodiscard]] bool
    findWriteElem(WriteData &wData, const QString &elemName, T &oVal);

  //==================================file===============================
  // parsers
  [[nodiscard]] RaceLogInfo readXMLLog(bool isRace);
  [[nodiscard]] QMap<QString, QString> readHDV();
  [[nodiscard]] QMap<QString, QString> readVEH();
  [[nodiscard]] QVector<QMap<QString, QString>> readRCD();

  //=========================specific log parsing sub methods ==========
  // xml
  [[nodiscard]] QMap<QString, QString> processMainLog(QXmlStreamReader &xml);

  // parse incident elements
  [[nodiscard]] QVector<DriverPair> processIncidents(QXmlStreamReader &xml);

  // constructs vector of incidents without equal pairings
  [[nodiscard]] QVector<DriverPair>
    processEqualCombinations(const QVector<QString> &incindents) const;

  // parse drivers info
  [[nodiscard]] QVector<DriverInfo>
    processDrivers(QXmlStreamReader &xml, const QVector<QString> &seqData);

  // parse driver lap times
  [[nodiscard]] QVector<QPair<int, double>>
    processDriverLaps(QXmlStreamReader &xml);

  //===================================class data======================
  QString fileName;
  QString fileData;
  FileType fileType;
};


//===============================write mod file definition============
template<typename T>
bool Parser::updateModFileData(const QVector<WriteDataInput<T>> &input)
{
  if (!isModFile())
  {
    qDebug() << "write mod file: not a mod file type!";
    return false;
  }
  auto backupData = fileData;
  for (auto i : input)
  {
    QString oVal, nVal;// transform input values to string
    QTextStream valS(&oVal);
    valS << i.oldVal;
    valS.setString(&nVal);
    valS << i.newVal;
    // look for element
    int initIndex = -1;
    WriteData dataStruct{ QTextStream(&fileData), QString(), initIndex };
    if (!findWriteElem(dataStruct, i.elemName, oVal)) return false;
    // backup fileData member
    // insert the new element into fileData
    const auto originalLineLength = dataStruct.line.size();
    dataStruct.line.remove(dataStruct.index, oVal.size());
    dataStruct.line.insert(dataStruct.index, nVal);
    const auto lineStartIndex =
      static_cast<int>(dataStruct.data.pos()) - dataStruct.line.size() - 1;
    backupData.remove(lineStartIndex, originalLineLength);
    backupData.insert(lineStartIndex, dataStruct.line);
  }
  return doWriteModFile(backupData);
}

template<typename T>
bool Parser::findWriteElem(WriteData &wData, const QString &elemName, T &oVal)
{
  while (!wData.data.atEnd())
  {
    wData.line = wData.data.readLine();
    if (!wData.line.isNull() && wData.line.contains(elemName))
    {
      wData.index = wData.line.indexOf(oVal);
      if (wData.index == -1)
      {
        qDebug() << "write mod file: couldnt find the value!";
        return false;
      }
      break;
    }
  }
  if (wData.data.atEnd() && wData.index == -1)
  {
    qDebug() << "write mod file: couldnt find the element!";
    return false;
  }
  return true;
}

#endif// Parser_H
