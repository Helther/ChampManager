#ifndef parser_H
#define parser_H
#include <parserConsts.h>
using namespace parserConsts;
using namespace FileTypes;

inline const int MAX_PARSE_THREAD_COUNT = QThread::idealThreadCount() - 1;

// input data for write elem finder
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
//========================= Parser hierarchy =====================//
// classes that hold file of certain xml logs and settings config types
// for the purposes of reading them and represention it's data in
// in a convenient form for database, or for writing new values
//=======
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


  //=============================getters===========================/
  [[nodiscard]] constexpr FileType getFileType() const { return fileType; }
  [[nodiscard]] const QString getFileData() const { return fileData; }
  [[nodiscard]] const QString getFileName() const { return fileName; }

  void setFileData(const QString &inData) { fileData = inData; }

protected:
  // read file for all info, depending on filetype and its data
  virtual QVariant readFileContent() = 0;

  //==============class data==================//
  QString fileName;
  QString fileData;
  FileType fileType;

private:
  // just opens file with error catching
  bool openFile(QFile &file, const QIODevice::OpenMode &mode);
};


class XmlParser : public Parser
{
public:
  [[nodiscard]] RaceLogInfo getParseData()
  {
    return readFileContent().value<RaceLogInfo>();
  }


protected:
  explicit XmlParser(QFile &file) : Parser(file) { fileType = readFileType(); }

  QVariant readFileContent() override;

  [[nodiscard]] FileType readFileType();

  [[nodiscard]] virtual RaceLogInfo
    readXMLLog(const QVector<QString> &ListOfElems);

protected:
  // parse incident elements
  QVector<StringPair> processIncidents(QXmlStreamReader &xml, bool driversOnly);

  // parse drivers info
  QVector<DriverInfo> processDrivers(QXmlStreamReader &xml,
                                     const QVector<QString> &seqData);

  QVector<QPair<QString, QString>> processMainLog(QXmlStreamReader &xml);

  // reads through file using xml reader reference in search of given element
  // name, returns element value, if never found returns null qstring
  QString findXMLElement(QXmlStreamReader &xml,
                         const QString &elemName,
                         const QString &stopEndElem = "atEnd",
                         bool okIfDidntFind = false,
                         bool stopStartElem = false);

private:
  // constructs vector of incidents without equal pairings
  QVector<StringPair>
    processEqualCombinations(const QVector<QString> &incindents) const;


  // parse driver lap times
  QPair<QString, QVector<QPair<int, double>>>
    processDriverLaps(QXmlStreamReader &xml);

  // creates csv string of lap times for db
  QString generateLapData(QVector<QPair<int, double>> data);
  QVector<StringPair>
    parseIncDriverName(const QVector<QString> &incindents) const;
};


class PXmlParser : public XmlParser
{
public:
  explicit PXmlParser(QFile &file) : XmlParser(file)
  {
    if (!(fileType == FileType::PracticeLog))
      throw std::runtime_error("fileType check error wrong file type");
  }
};


class QXmlParser : public XmlParser
{
public:
  explicit QXmlParser(QFile &file) : XmlParser(file)
  {
    if (!(fileType == FileType::QualiLog))
      throw std::runtime_error("fileType check error wrong file type");
  }
};


class RXmlParser : public XmlParser
{
public:
  explicit RXmlParser(QFile &file) : XmlParser(file)
  {
    if (fileType != FileType::RaceLog)
      throw std::runtime_error("fileType check error wrong file type");
  }

protected:
  QVariant readFileContent() override;
};


class DataSetParser : public XmlParser
{
public:
  explicit DataSetParser(QFile &file) : XmlParser(file) {}
  RaceLogInfo readXMLLog(const QVector<QString> &ListOfElems) override;
  // produces csv file with data given pairs of session type / drivers data
  static bool convertToCSV(const QVector<QPair<QString, RaceLogInfo>> &dataSet,
                           QFile &oFile);

private:
  // turns data elements to lines of csv's
  static QVector<QString>
    preprocessDataSet(const QPair<QString, RaceLogInfo> &dataSet,
                      bool displayHeaders);
  static QString makeSCVLine(const QVector<QString> &data);
  static bool hasIncidents(const QString &driver,
                           const QVector<StringPair> &incidents);
};


class ModParser : public Parser
{
public:
  explicit ModParser(QFile &file) : Parser(file) { fileType = readFileType(); }


  // looks for values into specific element names given names/values list
  template<typename T>
  bool updateModFileData(const QVector<WriteDataInput<T>> &input);

private:
  [[nodiscard]] FileType readFileType();
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
    if (fileType != FileType::RCD)
      throw std::runtime_error("fileType check error wrong file type");
  }
  QVariant readFileContent() override;

  [[nodiscard]] DriverStats getParseData()
  {
    return readFileContent().value<DriverStats>();
  }

private:
  DriverStats readRCD();
};


class VEHParser : public ModParser
{
public:
  explicit VEHParser(QFile &file) : ModParser(file)
  {
    if (fileType != FileType::VEH)
      throw std::runtime_error("fileType check error wrong file type");
  }
  QVariant readFileContent() override;

  [[nodiscard]] QVector<StringPair> getParseData()
  {
    return readFileContent().value<QVector<StringPair>>();
  }

private:
  QVector<StringPair> readVEH();
};


class HDVParser : public ModParser
{
public:
  explicit HDVParser(QFile &file) : ModParser(file)
  {
    if (fileType != FileType::HDV)
      throw std::runtime_error("fileType check error wrong file type");
  }
  QVariant readFileContent() override;

  [[nodiscard]] QVector<StringPair> getParseData()
  {
    return readFileContent().value<QVector<StringPair>>();
  }

private:
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
