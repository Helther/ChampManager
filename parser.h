#ifndef PARSER_H
#define PARSER_H

#include <QtCore>

using DriverPair = QPair<QString,QString>;

enum class FileType
{
    RaceLog = 0,
    QualiLog,
    Practice,
    RCD,
    HDV,
    VEH,
    Error
};

struct DriverInfo
{
    DriverInfo();
    //добавить методы для цикличн парса по типам
    QMap<uint, QPair<QString,QString>> stringValues;
    QMap<uint, QPair<QString,uint>> uintValues;
    QString name,
            vehFile,
            vehName,
            teamName,
            carType,
            carClass,
            carNumber,
            finishedStatus;
    uint position,
         classPosition,
         gridPos,
         classGridPos,
         pitstopsNum;
    double bestLapTime;
    QList<QPair<uint,double>> lapTimes;
};
struct LogInfo
{

/*
PlayerFile
Mod
TrackVenue
Laps
Stream/Incident - parse incidents
Driver - parse list of drivers
    Name
    VehFile
    VehName
    CarType
    CarClass
    CarNumber
    TeamName
    GridPos
    Position
    ClassGridPos
    ClassPosition
    Lap - list of  lap times
    BestLapTime
    Pitstops
    FinishStatus
    DNFReason
*/

    QString playerName,
            modName,
            trackName;
    uint numberOfLaps;
    FileType logType;
    QList<DriverInfo> drivers;
    //race log
    //Stream/Incident - parse incidents
    QVector<DriverPair> incidents;
};


class parser
{
public:
    parser(const QString& fileName);

    //just opens file with error catching
    bool openFile(QFile& file,const QIODevice::OpenMode& mode);
    // straightforward name
    static bool backupFile(const QString& filePath, const QString& backupPath,
                           const QString& backupName = "_backup");
    // finds type of file
    FileType GetFileType();

    //////////////////////parsing methods for xml logs////////////////////////////

    //parse sequence of parameters to vectors of data
    //main parsing methods
    LogInfo ReadXMLLog();
    //specific parsing sub methods
    void Incidents(LogInfo& log);
    //parse general log info
    void MainInfo(LogInfo& log);
    //parse drivers info
    void DriverMain(LogInfo& log);
    //parse driver lap times
    QList<QPair<uint,double>> DriverLaps();

    /////////////////// parse config files/////////////////////////////
    ///read and write different types of config files
private:
    QString fileName;
    QString fileData;
    FileType fileType;
};
#endif // PARSER_H


/*
Veh file
    DefaultLivery
    HDVehicle
    Driver
    Team
    Engine
    Classes
    Category
HDV file
    [FRONTWING]
        FWSetting=34
        FWDragParams=(0.07432, 0.00313, 0.000012)
        FWLiftParams=(-0.2902,-0.0116, 0.000026)
    [REARWING]
        RWSetting=32
        RWDragParams=(0.08001, 0.00770, 0.000022)
        RWLiftParams=(-0.3224,-0.0141, 0.000064)
    [BODYAERO]
        BodyDragBase=(0.3112)
        RadiatorDrag
        RadiatorLift
        BrakeDuctDrag
        BrakeDuctLift
    [DIFFUSER]
        DiffuserBasePlus=(-1.2995, 0.01,0.10, 0.90) // Base lift and Half/1st/2nd order with rear ride height
        DiffuserDraftLiftMult=1.0          // Effect of draft on diffuser's lift response
        DiffuserSideways=(0.232)            // Dropoff with yaw (0.0 = none, 1.0 = max)

    [ENGINE]
        GeneralTorqueMult*=1.0
        GeneralPowerMult*=1.01141
        GeneralEngineBrakeMult*=1.0
RCD file
    ASR_F1_2018
    {
     Lance Stroll
      {
        Aggression = 50.00
        //Reputation = 50.00
        //Courtesy = 42.00
        Composure = 49.00 (if AI Mistakes is > 0 in the playerfile)
        Speed = 77.50
        //QualifySpeed = 78.5
        //WetSpeed = 55.0
        StartSkill = 92.0
        Crash = 10.50 Only used in auto completing laps
        //Recovery = 82.00
        //CompletedLaps = 86.00 Only used in auto completing laps
        MinRacingSkill = 65.50
      }
    }

 */

///////for writing files
/*
QFile test("./ASR_F1_2018.rcd");
parser::openFile(test, QIODevice::ReadWrite);
QString data = test.readAll();
QString name = "Lance Stroll";
auto index = data.indexOf(name);
data.remove(index,name.size());
data.insert(index,"Fernando Alonso");
test.seek(0);
if(test.write(data.toUtf8()) == -1)
{
    qDebug()<<"write failed";
}
test.close();
*/
