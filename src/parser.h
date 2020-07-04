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

struct RaceLogInfo
{
  QMap<QString, QString> SeqElems;
  QVector<DriverInfo> drivers;
  QVector<DriverPair> incidents;
};

inline QDebug operator<<(QDebug debug, const RaceLogInfo &log)
{
  auto it = log.SeqElems.cbegin();
  QDebugStateSaver saver(debug);
  debug.nospace() << "==========xml log data========\n";
  debug.nospace() << "==========Incidents============\n";
  for (const auto &i : log.incidents)
    debug.nospace() << "Incident between " << i.first << " and " << i.second
                    << '\n';
  while (it != log.SeqElems.cend())
  {
    debug.nospace() << it.key() << " " << it.value() << '\n';
    ++it;
  }
  for (const auto &i : log.drivers) debug << i;
  return debug;
}

class Parser
{
public:
  explicit Parser(QFile &file);
  ~Parser();

  // straightforward name, gives backup a name with a current date
  // returns bool result and full backup file path
  // deletes backup if not successful
  static BackupData backupFile(const QString &filePath,
                               const QString &backupPath) noexcept;
  static bool restoreFile(const QString &filePath,
                          const QString &backupPath) noexcept;

  // read file for all info, depending on filetype
  bool readFileContent();
  // write values into specific element names given names/values list
  template<typename T>
  [[nodiscard]] bool writeModFile(const QString &elemName, T oldVal, T newVal);
  /// todo: maybe add multiple vals support

  //=============================getters===========================/
  [[nodiscard]] constexpr FileType getFileType() const { return fileType; }
  [[nodiscard]] const QString getFileData() const { return fileData; }
  [[nodiscard]] const QString getFileName() const { return fileName; }

  void setFileData(const QString &inData) { fileData = inData; }

private:
  // set a parsing error
  void raiseError(const QString &msg);

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
  FileType fileType = FileType::Error;
  bool hasError{ false };
  QString errorMessage;
};


//===============================write mod file definition============
template<typename T>
[[nodiscard]] bool
  Parser::writeModFile(const QString &elemName, T oldVal, T newVal)
{
  if (!isModFile())
  {
    qDebug() << "write mod file: not a mod file type!";
    return false;
  }
  QString oVal, nVal;// transform input values to string
  QTextStream valS(&oVal);
  valS << oldVal;
  valS.setString(&nVal);
  valS << newVal;
  QTextStream data(&fileData);// look for element in the file line by line
  QString line;
  int index = -1;
  while (!data.atEnd())
  {
    line = data.readLine();
    if (!line.isNull() && line.contains(elemName))
    {
      index = line.indexOf(oVal);
      if (index == -1)
      {
        qDebug() << "write mod file: couldnt find the value!";
        return false;
      }
      break;
    }
  }
  if (data.atEnd() && index == -1)
  {
    qDebug() << "write mod file: couldnt find the element!";
    return false;
  }
  // insert the new element into fileData
  auto originalLineLength = line.size();
  line.remove(index, oVal.size());
  line.insert(index, nVal);
  auto lineStartIndex = static_cast<int>(data.pos()) - line.size() - 1;
  fileData.remove(lineStartIndex, originalLineLength);
  fileData.insert(lineStartIndex, line);
  // backup before write, then open and try to write, if failed try to restore
  /// temp backup path
  auto pathWOname = fileName;
  pathWOname = pathWOname.remove(fileName.split('/').last());
  BackupData backUpRes{ backupFile(fileName, pathWOname) };
  if (!backUpRes.result)
  {
    qDebug() << "write mod file: couldnt back up the file";
    return false;
  }
  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly)
      && file.write(fileData.toStdString().c_str()) != -1)
  {
    qDebug() << "write mod file: success!";
    QFile bFile(backUpRes.fileFullPath);// delete backup
    if (bFile.exists()) bFile.remove();
    return true;
  } else
  {
    // try to restore from backup
    restoreFile(fileName, backUpRes.fileFullPath);
    qDebug() << "write mod file: couldnt open or write to the file";
    return false;
  }
}

#endif// Parser_H
