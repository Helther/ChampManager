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
  for (const auto i : dr.lapTimes)
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
  for (const auto i : log.incidents)
    debug.nospace() << "Incident between " << i.first << " and " << i.second
                    << '\n';
  while (it != log.SeqElems.cend())
  {
    debug.nospace() << it.key() << " " << it.value() << '\n';
    ++it;
  }
  for (const auto i : log.drivers) debug << i;
  return debug;
}
class Parser
{
public:
  Parser(const QString &fileName);
  ~Parser();

  // straightforward name, gives backup a name with a current date
  // returns bool result and full backup file path
  static BackupData backupFile(const QString &filePath,
                               const QString &backupPath) noexcept;

  // read file for all info, depending on filetype
  bool readFileContent();

  // write values into specific element names given names/values list
  [[nodiscard]] bool writeModFile(QVector<QPair<QString, QString>> NamesValues);

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

  // reads through file using xml reader reference in search of given element
  // name
  // returns element value, if never found returns empty qstring
  QString findXMLElement(QXmlStreamReader &xml,
                         const QString &elemName,
                         const QString &stopEndElem = "atEnd",
                         bool okIfDidntFind = false,
                         bool stopStartElem = false);

  //==================================file===============================
  // parsers
  [[nodiscard]] RaceLogInfo readXMLLog(bool isRace);
  [[nodiscard]] QMap<QString, QString> readHDV() const;//////////TODO: define
  [[nodiscard]] QMap<QString, QString> readVEH() const;//////////TODO: define
  [[nodiscard]] QMap<QString, QString> readRCD() const;//////////TODO: define

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
  bool hasError{};
  QString errorMessage;
};
#endif// Parser_H
