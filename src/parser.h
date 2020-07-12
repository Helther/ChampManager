#ifndef parser_H
#define parser_H
#include <parserConsts.h>

using StringPair = QPair<QString, QString>;
using DriverStats = QVector<QVector<QPair<QString, QString>>>;

struct BackupData
{
  bool result = false;
  QString fileFullPath;
};

struct DriverInfo
{
  QVector<StringPair> SeqElems;
  QVector<QPair<int, double>> lapTimes;
};

#ifdef QT_DEBUG
inline QDebug operator<<(QDebug debug, const DriverInfo &dr)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "==========Driver data========\n";
  for (const auto &i : dr.SeqElems) debug << i.first << " " << i.second << '\n';
  debug.nospace() << "===========Laps==============\n";
  for (const auto &i : dr.lapTimes)
    debug.nospace() << "# " << i.first << " " << i.second << '\n';
  return debug;
}
#endif

struct RaceLogInfo
{
  QVector<StringPair> SeqElems;
  QVector<DriverInfo> drivers;
  QVector<StringPair> incidents;
};

#ifdef QT_DEBUG
inline QDebug operator<<(QDebug debug, const RaceLogInfo &log)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "==========xml log data========\n";
  debug.nospace() << "==========Incidents============\n";
  for (const auto &i : log.incidents)
    debug.nospace() << "Incident between " << i.first << " and " << i.second
                    << '\n';
  debug.nospace() << "==========Incidents END============\n";
  for (const auto &i : log.SeqElems)
    debug.nospace() << i.first << " " << i.second << '\n';
  for (const auto &i : log.drivers) debug << i;
  return debug;
}
#endif

struct WriteData
{
  QTextStream data;
  QString line;
  int index;
  int pad;
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
  virtual ~Parser();

  // straightforward name, gives backup a name with a current date
  // returns bool result and full backup file path
  // deletes backup and empy file path if not successful
  static BackupData backupFile(const QString &filePath,
                               const QString &backupPath) noexcept;
  static bool restoreFile(const QString &filePath,
                          const QString &backupPath) noexcept;

  // read file for all info, depending on filetype
  virtual bool readFileContent() = 0;

  //=============================getters===========================/
  [[nodiscard]] constexpr FileType getFileType() const { return fileType; }
  [[nodiscard]] const QString getFileData() const { return fileData; }
  [[nodiscard]] const QString getFileName() const { return fileName; }

  void setFileData(const QString &inData) { fileData = inData; }

protected:
  //[[nodiscard]] virtual FileType readFileType() = 0;
  //[[nodiscard]] virtual bool checkFileType() = 0;
  //===================================class data======================
  QString fileName;
  QString fileData;
  FileType fileType;

private:
  // just opens file with error catching
  [[nodiscard]] bool openFile(QFile &file, const QIODevice::OpenMode &mode);
};

class XmlParser : public Parser
{
public:
  explicit XmlParser(QFile &file) : Parser(file) { fileType = readFileType(); }
  // reads through file using xml reader reference in search of given element
  // name, returns element value, if never found returns empty qstring
  QString findXMLElement(QXmlStreamReader &xml,
                         const QString &elemName,
                         const QString &stopEndElem = "atEnd",
                         bool okIfDidntFind = false,
                         bool stopStartElem = false);

protected:
  [[nodiscard]] FileType readFileType();

  //=========================specific log parsing sub methods ==========
  // xml
  [[nodiscard]] RaceLogInfo readXMLLog(const QVector<QString> &ListOfElems);

private:
  [[nodiscard]] QVector<QPair<QString, QString>>
    processMainLog(QXmlStreamReader &xml);

  // parse incident elements
  [[nodiscard]] QVector<StringPair> processIncidents(QXmlStreamReader &xml);

  // constructs vector of incidents without equal pairings
  [[nodiscard]] QVector<StringPair>
    processEqualCombinations(const QVector<QString> &incindents) const;

  // parse drivers info
  [[nodiscard]] QVector<DriverInfo>
    processDrivers(QXmlStreamReader &xml, const QVector<QString> &seqData);

  // parse driver lap times
  [[nodiscard]] QVector<QPair<int, double>>
    processDriverLaps(QXmlStreamReader &xml);
};

class PQXmlParser : public XmlParser
{
public:
  explicit PQXmlParser(QFile &file) : XmlParser(file)
  {
    if (!checkFileType())
      throw std::runtime_error("fileType check error wrong file type");
  }
  bool readFileContent() override;

private:
  bool checkFileType();
};

class RXmlParser : public XmlParser
{
public:
  explicit RXmlParser(QFile &file) : XmlParser(file)
  {
    if (!checkFileType())
      throw std::runtime_error("fileType check error wrong file type");
  }
  bool readFileContent() override;

private:
  bool checkFileType();
};

class ModParser : public Parser
{
public:
  explicit ModParser(QFile &file) : Parser(file) { fileType = readFileType(); }

  [[nodiscard]] FileType readFileType();
  // looks for values into specific element names given names/values list
  template<typename T>
  bool updateModFileData(const QVector<WriteDataInput<T>> &input);

private:
  // writes resulting changes to a file
  bool doWriteModFile(const QString &data);
  // looks for element with a given name and value, returns true if successful
  template<typename T>
  [[nodiscard]] bool
    findWriteElem(WriteData &wData, const QString &elemName, const T &oVal);
};

class RCDParser : public ModParser
{
public:
  explicit RCDParser(QFile &file) : ModParser(file)
  {
    if (!checkFileType())
      throw std::runtime_error("fileType check error wrong file type");
  }
  bool readFileContent() override;

private:
  bool checkFileType();
  DriverStats readRCD();
};

class VEHParser : public ModParser
{
public:
  explicit VEHParser(QFile &file) : ModParser(file)
  {
    if (!checkFileType())
      throw std::runtime_error("fileType check error wrong file type");
  }
  bool readFileContent() override;

private:
  bool checkFileType();
  QVector<StringPair> readVEH();
};

class HDVParser : public ModParser
{
public:
  explicit HDVParser(QFile &file) : ModParser(file)
  {
    if (!checkFileType())
      throw std::runtime_error("fileType check error wrong file type");
  }
  bool readFileContent() override;

private:
  bool checkFileType();
  QVector<StringPair> readHDV();
};

//===============================write mod file definition============
template<typename T>
bool ModParser::updateModFileData(const QVector<WriteDataInput<T>> &input)
{
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
    WriteData dataStruct{ QTextStream(&fileData), QString(), initIndex, 0 };
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
bool ModParser::findWriteElem(WriteData &wData,
                              const QString &elemName,
                              const T &oVal)
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
