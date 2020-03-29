#ifndef parser_H
#define parser_H

#include <QtCore>

using DriverPair = QPair<QString,QString>;

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

struct SeqDataStruct
{
    //names of xml elements
    //all values would be strings, except when specified
    const QVector<QString> PraciceLogElements{
        "PlayerFile",
        "Mod",
        "TrackVenue",
        "Laps",//int
        "Driver"//vector
    };
    const QVector<QString> QualiLogElements{
        "PlayerFile",
        "Mod",
        "TrackVenue",
        "Laps",//int
        "Driver"//vector
    };
    const QVector<QString> RaceLogElements{
        "PlayerFile",
        "Mod",
        "TrackVenue",
        "Laps",//int
        "Stream",//vector
        "Driver"//vector
    };
    const QVector<QString> DriversElements{
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
        "FinishStatus",
        "DNFReason"
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
};

struct DriverInfo
{
    QMap<QString,QString> SeqElems;
    QVector<QPair<int,double>> lapTimes;
};

struct RaceLogInfo
{
    QMap<QString,QString> SeqElems;
    QVector<DriverInfo> drivers;
    QVector<DriverPair> incidents;
};

struct PractQualiLogInfo
{
    QMap<QString,QString> SeqElems;
    QVector<DriverInfo> drivers;
};


class Parser
{
public:
    Parser(const QString& fileName);

    // straightforward name
    static bool backupFile(const QString& filePath, const QString& backupPath,
                           const QString& backupName);

    //read file for all info, depending on filetype
    bool readFileContent();
    //write values into specific element names given names/values list
    bool writeModFile(QVector<QPair<QString,QString>> NamesValues);

    /////////////////////////////getters/////////////////
    inline FileType getFileType() const {return fileType;}
    inline const QString getFileData() const {return fileData;}
    inline QString& getFileData() {return fileData;}
    inline const QString getFileName() const {return fileName;}
    //inline QString& getFileName() {return fileName;}

private:

    //just opens file with error catching
    bool openFile(QFile& file,const QIODevice::OpenMode& mode);
    // finds type of file
    FileType readFileType();
    //
    const QString findXMLElement(QXmlStreamReader& xml,const QString& elemName);
    //log parsers
    PractQualiLogInfo readPractQualiLog();
    RaceLogInfo readRaceLog();
    QMap<QString,QString> readHDV();
    QMap<QString,QString> readVEH();
    QMap<QString,QString> readRCD();

    //specific parsing sub methods
    //parse incident elements
    QVector<DriverPair> Incidents(QXmlStreamReader& xml);
    //parse drivers info
    QVector<DriverInfo> Drivers(QXmlStreamReader& xml);
    //parse driver lap times
    QVector<QPair<int,double>> DriverLaps(QXmlStreamReader& xml);

    /////////////////data/////////////////
    QString fileName;
    QString fileData;
    FileType fileType;
};
#endif // Parser_H


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
    BestLapTime - double
    Pitstops
    FinishStatus
    DNFReason
*/
///////for writing files
/*
QFile test("./ASR_F1_2018.rcd");
Parser::openFile(test, QIODevice::ReadWrite);
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
