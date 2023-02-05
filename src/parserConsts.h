#pragma once

#include <QtCore>


using StringPair = QPair<QString, QString>;

struct HDVData
{
  QString prefix;
  QString name;
  QString value;
  bool operator==(const HDVData& other) const
  { return prefix == other.prefix && name == other.name && value == other.value; }
};
Q_DECLARE_METATYPE(HDVData)

enum class HDVEntries {
  FWSetting = 0,
  FWDragParams,
  FWLiftParams,
  RWSetting,
  RWDragParams,
  RWLiftParams,
  BodyDragBase,
  DiffuserBasePlus,
  GeneralPowerMult
};

using HDVParams = QMap<HDVEntries, HDVData>;
Q_DECLARE_METATYPE(HDVParams)


inline constexpr double RCD_DEFAULT_VALUE = 100.;

struct RCDData
{
  QString name;
  double value;
};

enum class RCDEntries
{
  Aggression = 0,
  Composure,
  Speed,
  StartSkill,
  MinRacingSkill
};

Q_DECLARE_METATYPE(RCDData)

using RCDParams = QMap<RCDEntries, RCDData>;

using DriverStats = QMap<QString, RCDParams>;
Q_DECLARE_METATYPE(DriverStats)


// backup function return type
struct BackupData
{
  bool result = false;
  QString fileFullPath;
};

// part of xmlLog parse results
struct DriverInfo
{
  QVector<StringPair> SeqElems;
  QVector<QPair<int, double>> lapTimes;
};

inline QString &operator<<(QString &retString, const DriverInfo &dr)
{
  QTextStream s(&retString);
  s << "==========Driver data========\n";
  for (const auto &i : dr.SeqElems) s << i.first << " " << i.second << '\n';
  s << "===========Laps==============\n";
  for (const auto &i : dr.lapTimes)
    s << "# " << i.first << " " << i.second << '\n';
  return retString;
}

// xmlLog results
struct RaceLogInfo
{
  QVector<StringPair> SeqElems;
  QVector<DriverInfo> drivers;
  QVector<StringPair> incidents;
};
Q_DECLARE_METATYPE(RaceLogInfo)

inline QString &operator<<(QString &retString, const RaceLogInfo &log)
{
  QTextStream s(&retString);
  s << "==========xml log data========\n";
  s << "==========Incidents============\n";
  for (const auto &i : log.incidents)
    s << "Incident between " << i.first << " and " << i.second << '\n';
  s << "==========Incidents END============\n";
  for (const auto &i : log.SeqElems) s << i.first << " " << i.second << '\n';
  for (const auto &i : log.drivers) retString << i;
  return retString;
}

inline QString &operator<<(QString &retString, const HDVParams &data)
{
  QTextStream s(&retString);
  s << "==========HDV data========" << Qt::endl;
  for (const auto &i : data)
    s << i.prefix << Qt::endl << i.name << '=' << i.value << Qt::endl;
  return retString;
}

inline QString &operator<<(QString &retString, const DriverStats &data)
{
  QTextStream s(&retString);
  s << "==========RCD data========\n";
  for (auto i = data.begin(); i != data.end(); i++)
  {
    s << i.key() << Qt::endl;
    for (const auto &j : i.value()) s << j.name << "=" << j.value << Qt::endl;
    s << Qt::endl;
  }
  return retString;
}

[[nodiscard]] inline QString getVEHDataString(const QVector<StringPair> &data)
{
  QString retString;
  QTextStream s(&retString);
  s << "==========VEH data========\n";
  for (const auto &i : data) s << i.first << "=" << i.second << '\n';
  return retString;
}


namespace parserConsts {

namespace seqElems {

  // names of xml elements
  // all values would be strings, except when specified
  inline const QVector<QString> MainLogElements{
    "PlayerFile",// str
    "TimeString",// str
    "Mod",// str
    "TrackCourse",
    "RaceLaps"// int
  };
  inline const QVector<QString> DriversRaceElements{
    "Name",// str
    "VehFile",// str
    "VehName",// str
    "CarType",// str
    "CarClass",// str
    "CarNumber",// str
    "TeamName",// str
    "GridPos",// int
    "Position",// int
    "ClassGridPos",// int
    "ClassPosition",// int
    "Lap",// vector of lap times(in sql store as a string)
    "BestLapTime",// double
    "Pitstops",// int
    "FinishStatus",// values None, Finished Normally, DNF
    //"DNFReason"
  };
  inline const QVector<QString> DataSetRElems{ "Name",
                                               "TeamName",
                                               "GridPos",
                                               "Position" };
  inline const QVector<QString> DataSetPQElems{ "Name",
                                                "TeamName",
                                                "Position" };

  inline const QVector<QString> DriversPQElements{
    "Name",// str
    "VehFile",// str
    "VehName",// str
    "CarType",// str
    "CarClass",// str
    "CarNumber",// str
    "TeamName",// str
    "Position",// int
    "ClassPosition",// int
    "Lap",// vector of lap times
    "BestLapTime",// double
  };


  inline const RCDParams RCDElements{
    {RCDEntries::Aggression, {"Aggression", RCD_DEFAULT_VALUE}},
    {RCDEntries::Composure, {"Composure", RCD_DEFAULT_VALUE}},
    {RCDEntries::Speed, {"Speed", RCD_DEFAULT_VALUE}},
    {RCDEntries::StartSkill, {"StartSkill", RCD_DEFAULT_VALUE}},
    {RCDEntries::MinRacingSkill, {"MinRacingSkill", RCD_DEFAULT_VALUE}}
  };

  inline const HDVParams HDVElements{
    {HDVEntries::FWSetting, {"[FRONTWING]", "FWSetting", ""}},// int
    {HDVEntries::FWDragParams, {"[FRONTWING]", "FWDragParams", ""}},// double
    {HDVEntries::FWLiftParams, {"[FRONTWING]", "FWLiftParams", ""}},// double
    {HDVEntries::RWSetting, {"[REARWING]", "RWSetting", ""}},// int
    {HDVEntries::RWDragParams, {"[REARWING]", "RWDragParams", ""}},// double
    {HDVEntries::RWLiftParams, {"[REARWING]", "RWLiftParams", ""}},// double
    {HDVEntries::BodyDragBase, {"[BODYAERO]", "BodyDragBase", ""}},// double
    //"RadiatorDrag",// double
    //"RadiatorLift",// double
    //"BrakeDuctDrag",// double
    //"BrakeDuctLift",// double
    {HDVEntries::DiffuserBasePlus, {"[DIFFUSER]", "DiffuserBasePlus", ""}},// double
    //"DiffuserDraftLiftMult",// double
    //"DiffuserSideways",// double
    //"GeneralTorqueMult",// double
    {HDVEntries::GeneralPowerMult, {"[ENGINE]", "GeneralPowerMult", ""}}// double
    //"GeneralEngineBrakeMult"// double
  };

  inline const QVector<QString> VEHElements{
    "HDVehicle",// str
    "Upgrades",// str
    "Driver",// str surrounded with ""
    "Classes"// str surrounded with ""
  };
  enum VEHElemIndexes
  {
    HDVehicle = 0,
    Upgrades,
    Driver,
    Classes
  };

}// namespace seqElems
namespace FileTypes {
  enum class FileType {
    RaceLog = 0,
    QualiLog,
    PracticeLog,
    RCD,
    HDV,
    VEH,
    INI,
    Error
  };

  inline QVector<QString> typeNames{ "Race", "Qualify", "Practice", "rcd",
                                     "hdv", "veh", "ini", "Error" };

  inline QString getFileTypeById(FileType type)
  {
    return typeNames.at(static_cast<int>(type));
  }
}
namespace INIFile {
  inline const QString tokenUpgradeType = "UpgradeType";
  inline const QString tokenUpgradeLevel = "UpgradeLevel";
  inline const QString tokenUpgradeTypeName = "\"CMM Perfomance\"";
  inline const QString tokenUpgradeLevelName = "\"Current\"";
  inline const QString elemPrefix = "HDV=";
  inline const char tokenValBegin = '=';
  inline const char tokenOpen = '{';
  inline const char tokenClose = '}';
}
}// namespace parserConsts



/// todo temp for debug
#include <chrono>
class Perf
{
public:
  Perf(const QString &inMsg)
    : msg(inMsg), startTime(std::chrono::high_resolution_clock::now())
  {}
  ~Perf()
  {
    const auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      endTime - startTime);
    qDebug() << msg << " duration in mcS: " << duration.count();
  }

private:
  QString msg;
  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

