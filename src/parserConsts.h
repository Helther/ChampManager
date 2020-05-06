#ifndef PARSERCONSTS_H
#define PARSERCONSTS_H

#include <QtCore>

enum class FileType {
  RaceLog = 1,
  QualiLog = 2,
  PracticeLog = 4,
  RCD = 8,
  HDV = 16,
  VEH = 32,
  Error = 64
};

namespace parserConsts {
namespace seqElems {

  // names of xml elements
  // all values would be strings, except when specified
  const QVector<QString> MainLogElements{
    "PlayerFile",// str
    "Mod",// str
    "TrackVenue",
    "RaceLaps"// int
  };
  const QVector<QString> DriversRaceElements{
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
  const QVector<QString> DriversPQElements{
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
  const QVector<QString> RCDElements{
    "Aggression",// double
    "Composure",// double
    "Speed",// double
    "StartSkill",// double
    "MinRacingSkill"// double
  };
  const QVector<QString> HDVElements{
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

  const QVector<QString> VEHElements{
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
  const QString Racetype = "Race";
  const QString Qualtype = "Qualify";
  const QString Practtype = "Practice";
  const QString rcdFile = "rcd";
  const QString hdvFile = "HDV";
  const QString vehFile = "veh";
}// namespace typeName
}// namespace parserConsts
#endif// PARSERCONSTS_H
