#ifndef PARSERCONSTS_H
#define PARSERCONSTS_H
#include <QtCore>

using StringPair = QPair<QString, QString>;
using DriverStats = QVector<QVector<StringPair>>;


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
inline QString getRCDDataString(const DriverStats &data)
{
  QString retString;
  QTextStream s(&retString);
  s << "==========RCD data========\n";
  for (const auto &i : data)
  {
    for (const auto &j : i) s << j.first << " " << j.second << '\n';
  }
  return retString;
}

inline QString getVEHDataString(const QVector<StringPair> &data)
{
  QString retString;
  QTextStream s(&retString);
  s << "==========VEH data========\n";
  for (const auto &i : data) qDebug() << i.first << " " << i.second << '\n';
  return retString;
}

inline QString getHDVDataString(const QVector<StringPair> &data)
{
  QString retString;
  QTextStream s(&retString);
  s << "==========HDV data========\n";
  for (const auto &i : data) qDebug() << i.first << " " << i.second << '\n';
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
  inline const QVector<QString> RCDElements{
    "Aggression",// double
    "Composure",// double
    "Speed",// double
    "StartSkill",// double
    "MinRacingSkill"// double
  };
  inline const QVector<QString> HDVElements{
    "FWSetting",// int
    "FWDragParams",// double
    "FWLiftParams",// double
    "RWSetting",// int
    "RWDragParams",// double
    "RWLiftParams",// double
    "BodyDragBase",// double
    "RadiatorDrag",// double
    "RadiatorLift",// double
    "BrakeDuctDrag",// double
    "BrakeDuctLift",// double
    "DiffuserBasePlus",// double
    "DiffuserDraftLiftMult",// double
    "DiffuserSideways",// double
    "GeneralTorqueMult",// double
    "GeneralPowerMult",// double
    "GeneralEngineBrakeMult"// double
  };

  inline const QVector<QString> VEHElements{
    "DefaultLivery",// str
    "HDVehicle",// str
    "Team",// str
    "Driver",// str
    "Engine",// str
    "Classes",// str
    "Category"// str
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
    Error
  };

  inline QVector<QString> typeNames{ "Race", "Qualify", "Practice",
                                     "rcd",  "HDV",     "veh" };

  inline QString getFileTypeById(FileType type)
  {
    return typeNames.at(static_cast<int>(type));
  }
}
}// namespace parserConsts

/// todo debug
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

#endif// PARSERCONSTS_H
