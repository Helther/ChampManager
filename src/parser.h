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
    const QVector<QString> MainLogElements{
        "PlayerFile",
        "Mod",
        "TrackVenue",
        "RaceLaps",//int
        "Stream",//vector
        "Driver"//vector
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
        "FinishStatus",
        "DNFReason"
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


class Parser
{
public:
    Parser(const QString& fileName);
    ~Parser();
    // straightforward name
    static bool backupFile(const QString& filePath, const QString& backupPath,
                           const QString& backupName);

    //read file for all info, depending on filetype
    bool readFileContent() const;
    //write values into specific element names given names/values list
    [[nodiscard]] bool writeModFile(QVector<QPair<QString,QString>> NamesValues);

    /////////////////////////////getters/////////////////
    [[nodiscard]] constexpr FileType getFileType() const {return fileType;}
    [[nodiscard]] const QString getFileData() const {return fileData;}
    [[nodiscard]] const QString getFileName() const {return fileName;}

    void setFileData(const QString& inData) {fileData = inData;}
private:
    //set a parsing error
    void raiseError(const QString& msg);
    //just opens file with error catching
    [[nodiscard]] bool openFile(QFile& file,const QIODevice::OpenMode& mode) const;
    // finds type of file
    [[nodiscard]] FileType readFileType();
    [[nodiscard]] FileType readModFileType() const;//////////define
    //
    const QString findXMLElement(QXmlStreamReader& xml,const QString& elemName);
    //log parsers
    [[nodiscard]] RaceLogInfo readPractQualiLog() const;//////////define///////add error check at the begining
    [[nodiscard]] RaceLogInfo readRaceLog() const;//////////define
    [[nodiscard]] QMap<QString,QString> readHDV() const;//////////define
    [[nodiscard]] QMap<QString,QString> readVEH() const;//////////define
    [[nodiscard]] QMap<QString,QString> readRCD() const;//////////define

    //specific parsing sub methods
    //parse incident elements
    [[nodiscard]] QVector<DriverPair> processIncidents(QXmlStreamReader& xml) const;
    //constructs vector of incidents without equal pairings
    [[nodiscard]] QVector<DriverPair> processEqualCombinations(const QVector<QString>& incindents) const;
    //parse drivers info
    [[nodiscard]] QVector<DriverInfo> processDrivers(QXmlStreamReader& xml) const;
    //parse driver lap times
    [[nodiscard]] QVector<QPair<int,double>> processDriverLaps(QXmlStreamReader& xml) const;

    /////////////////data/////////////////
    QString fileName;
    QString fileData;
    FileType fileType;
    bool hasError{};
    QString errorMessage;
};
#endif // Parser_H
