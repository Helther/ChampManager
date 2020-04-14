#ifndef PARSERCONSTS_H
#define PARSERCONSTS_H

#include <QtCore>

enum class FileType
{
    RaceLog = 0,
    QualiLog,
    PracticeLog,
    RCD,
    HDV,
    VEH,
    Error
};

namespace parserConsts
{
    namespace seqElems
    {
        //names of xml elements
        //all values would be strings, except when specified
        const QVector<QString> MainLogElements{
            "PlayerFile",
            "Mod",
            "TrackVenue",
            "RaceLaps"//int
        };
        const QVector<QString> DriversRaceElements{
            "Name",
            "VehFile",
            "VehName",
            "CarType",
            "CarClass",
            "CarNumber",
            "TeamName",
            "GridPos", //int
            "Position", //int
            "ClassGridPos", //int
            "ClassPosition", //int
            "Lap", //vector of lap times
            "BestLapTime", //double
            "Pitstops", //int
            "FinishStatus",//////todo figure out how to parse dnf
           //"DNFReason"
        };
        const QVector<QString> DriversPQElements{
            "Name",
            "VehFile",
            "VehName",
            "CarType",
            "CarClass",
            "CarNumber",
            "TeamName",
            "Position", //int
            "ClassPosition", //int
            "Lap", //vector of lap times
            "BestLapTime", //double
        };
        const QVector<QString> RCDElements{
            "Aggression", //double
            "Composure", //double
            "Speed", //double
            "StartSkill", //double
            "MinRacingSkill" //double
        };
        const QVector<QString> HDVElements{
            "FWSetting", //int
            "FWDragParams", //double
            "FWLiftParams", //double
            "RWSetting", //int
            "RWDragParams", //double
            "RWLiftParams", //double
            "BodyDragBase", //double
            "RadiatorDrag", //double
            "RadiatorLift", //double
            "BrakeDuctDrag", //double
            "BrakeDuctLift", //double
            "DiffuserBasePlus", //double
            "DiffuserDraftLiftMult", //double
            "DiffuserSideways", //double
            "GeneralTorqueMult", //double
            "GeneralPowerMult", //double
            "GeneralEngineBrakeMult" //double
        };
        const QVector<QString> VEHElements{
            "DefaultLivery",
            "HDVehicle",
            "Driver",
            "Team",
            "Engine",
            "Classes",
            "Category"
        };
    }
    namespace typeName
    {
        const QString Racetype = "Race";
        const QString Qualtype = "Qualify";
        const QString Practtype = "Practice";
        const QString rcdFile = "rcd";
        const QString hdvFile = "HDV";
        const QString vehFile = "veh";
    }
}

#endif // PARSERCONSTS_H
