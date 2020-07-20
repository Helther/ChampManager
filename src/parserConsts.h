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

// xmlLog results
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

namespace parserConsts {
namespace seqElems {

  // names of xml elements
  // all values would be strings, except when specified
  inline const QVector<QString> MainLogElements{
    "PlayerFile",// str
    "Mod",// str
    "TrackVenue",
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
#endif// PARSERCONSTS_H
