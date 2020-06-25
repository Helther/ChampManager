#ifndef PARSERCONSTS_H
#define PARSERCONSTS_H

#include <QtCore>

enum class FileType {
  RaceLog = 0,
  QualiLog,
  PracticeLog,
  RCD,
  HDV,
  VEH,
  Error
};

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
    "Lap",// vector of lap times
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
namespace typeName {
  inline const QString Racetype = "Race";
  inline const QString Qualtype = "Qualify";
  inline const QString Practtype = "Practice";
  inline const QString rcdFile = "rcd";
  inline const QString hdvFile = "HDV";
  inline const QString vehFile = "veh";
}// namespace typeName
}// namespace parserConsts
#endif// PARSERCONSTS_H
