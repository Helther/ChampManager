#include <parser.h>
#include <QXmlStreamReader>
#include <QDateTime>
//#ifdef QT_DEBUG QT_NO_DEBUG_OUTPUT
//#endif
using namespace parserConsts;

Parser::Parser(const QString &FileName) : fileName(FileName)
{
    QFile file(FileName);
    if(openFile(file,QIODevice::ReadWrite))
    {
        auto buffer = file.readAll();
        fileData = QString(buffer).toUtf8();
        if(!hasError)
            fileType = readFileType();
    }
}
Parser::~Parser()
{
    QFile file(fileName);
    if(file.isOpen())
        file.close();
}

FileType Parser::readFileType()
{
    const auto tokenName = "FixedUpgrades";//elem prior to one needed
    //find log type token
    QXmlStreamReader xml(fileData);
    if(!xml.readNextStartElement())//is file xml file
    {
        qDebug()<<"file not xml\n";
        return readModFileType();
    }
    findXMLElement(xml,tokenName);
    for(int i = 0;i<4;++i)//x4 to get to type token
    {
        xml.readNext();
        if(xml.hasError())
        {
            raiseError(xml.errorString());
            return FileType::Error;
        }
    }
    if(xml.name() == typeName::Racetype)
    {
        qDebug() << "Race\n";
        return FileType::RaceLog;
    }
    if(xml.name() == typeName::Qualtype)
    {
        qDebug() << "Quali\n";
        return FileType::QualiLog;
    }
    if(xml.name().contains(typeName::Practtype))
    {
        qDebug() << "Practice\n";
        return FileType::PracticeLog;
    }
    raiseError("No valid log type was found");
    return FileType::Error;
}

FileType Parser::readModFileType()
{
    QFile file(fileName);
    auto fileinfo = file.fileName().split(".",QString::KeepEmptyParts);
    const QString& fileExt = fileinfo.at(fileinfo.size()-1);
    if(fileExt == typeName::rcdFile)
    {
        qDebug() << "rcd\n";
        return FileType::RCD;
    }
    if(fileExt == typeName::hdvFile)
    {
        qDebug() << "hdv\n";
        return FileType::HDV;
    }
    if(fileExt == typeName::vehFile)
    {
        qDebug() << "veh\n";
        return FileType::VEH;
    }
    raiseError("No valid file type was found");
    return FileType::Error;
}

QString Parser::findXMLElement(QXmlStreamReader& xml, const QString& elemName,
                                     const QString& stopEndElem,bool okIfDidntFind)
{
    QXmlStreamReader::TokenType token;
    QString result;
    bool reachedStop = false;
    while(!xml.atEnd() && !xml.hasError())
    {

        if(token == QXmlStreamReader::TokenType::EndElement && stopEndElem != "atEnd" &&
            xml.name() == stopEndElem)
        {
            reachedStop = true;
            qDebug()<< "findXMLElem: reached stop elem";
            break;
        }
        if(token == QXmlStreamReader::TokenType::StartElement && xml.name() == elemName)
        {
            if(!xml.attributes().isEmpty())//////todo: consider variable number of attributes in elements
                xml.readNext();
            result = xml.text().toString();
            break;
        }
        token = xml.readNext();
    }
    if(xml.hasError())
        raiseError(xml.errorString());
    if (!okIfDidntFind && (xml.atEnd() || reachedStop))
        raiseError("xml reader couldn't find the element: " + elemName);
    return result;
}

RaceLogInfo Parser::readPractQualiLog()
{
    QXmlStreamReader xml(fileData);
    RaceLogInfo result;
    result.incidents = processIncidents(xml);
    result.drivers = processDrivers(xml,parserConsts::seqElems::DriversPQElements);
    qDebug()<<result;
    return result;
}

RaceLogInfo Parser::readRaceLog()
{
    QXmlStreamReader xml(fileData);
    RaceLogInfo result;
    result.incidents = processIncidents(xml);
    return result;
}

bool Parser::openFile(QFile& file,const QIODevice::OpenMode& mode)
{
    if(file.exists())
    {
        if(!file.isOpen())
        {
            if(file.open(mode) | QIODevice::Text)
                return true;
            raiseError("Can't open file");
        }
        qDebug() << "Is already open!\n";
            return true;
    }
    raiseError("File doesn't exist");
    return false;
}

bool Parser::backupFile(const QString &filePath,
                        const QString &backupPath)
{
    QFile file(filePath);
    QFile bFile(backupPath);
    //set backup file new name
    //get formatted current date and time
    QString backupName{QDateTime::currentDateTime().date().toString() +
                       QDateTime::currentDateTime().time().toString()};
    for(auto it = backupName.begin(); it<backupName.end() ;++it)
    {
        if(*it == ':')
            *it='_';
    }
    backupName.remove(" ");
    //get file name format
    auto fileinfo = file.fileName().split(".",QString::KeepEmptyParts);
    const QString fileExt = fileinfo.at(fileinfo.size()-1);
    fileinfo = file.fileName().split("/",QString::KeepEmptyParts);
    fileinfo = fileinfo.at(fileinfo.size()-1).split(".",QString::KeepEmptyParts);
    const QString fileName = fileinfo.at(0);
    //set new name
    bFile.setFileName(QString(backupPath + fileName + backupName+ "." + fileExt));
    //open and write files
    if(file.open(QIODevice::ReadOnly))
    {
        if(bFile.open(QIODevice::WriteOnly))
        {
            if(bFile.write(file.readAll()) == -1)
            {
                bFile.remove();
                return false;
            }
            file.close();
            bFile.close();
            qDebug()<< "File " << file.fileName()
                    <<" backed up succesfully. New file is"
                   << bFile.fileName() << "\n";
        }
        else
            file.close();
    }
    return false;
}

bool Parser::readFileContent()/////////TODO: finish
{
    ///after reading call static methods for writing to db
    if(hasError)
    {
        qDebug()<<"readFile: "<<errorMessage<<'\n';
        return false;
    }
    switch(fileType)
    {
        case FileType::Error :
        {
            return false;
        }
        case FileType::HDV :
        {
            break;
        }
        case FileType::RCD :
        {
            break;
        }
        case FileType::VEH :
        {
            break;
        }
        case FileType::RaceLog :
        {
            auto data = readRaceLog();
            break;
        }
        case FileType::QualiLog :
        case FileType::PracticeLog :
        {
            auto data = readPractQualiLog();
            break;
        }
    }

    //catch excepts
    return false;
}

void Parser::raiseError(const QString& msg)
{
    if(!hasError)
    {
        if(!msg.isEmpty())
        {
            errorMessage = msg;
            hasError = true;
            qDebug()<<"raiseError: "<<errorMessage<<'\n';///for debug
        }
        else
        {
            qDebug()<<"raiseError: error msg is empty\n";
        }
    }
    else
        qDebug()<<"there is already an error: "<<errorMessage<<'\n';///for debug
}

QVector<DriverPair> Parser::processIncidents(QXmlStreamReader& xml)
{
    const QString XMLName = "Incident";
    const QString XMLEndName = "Stream";
    QVector<QString> incidents;
    //parse and save incident strings
    QString prevDriverName;
    while(!hasError && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement &&
             xml.name() == XMLEndName))
    {
        const auto currentElem = findXMLElement(xml,XMLName,XMLEndName,true);
        if(currentElem.isEmpty())//check if found something, continue if empty
            continue;
        //process two names involved in incident
        auto currentDriverName = currentElem.split("(",QString::SkipEmptyParts).first();
        if(prevDriverName != currentDriverName)
            incidents.push_back(currentElem);
        prevDriverName = currentDriverName;
    }
    if(incidents.isEmpty())//check if found something, return empty if not
        return QVector<DriverPair>{};
    return processEqualCombinations(incidents);
}

QVector<DriverPair> Parser::processEqualCombinations(const QVector<QString>& incindents) const
{
    QVector<DriverPair> incidentData;
    const QString incLineSplitter = "vehicle ";
    //ignore equal driver combinations
    QString currentString;
    const int minValidLineLength = 4;
    for(const auto& i : incindents)
    {
        currentString = i;
        auto stringList = currentString.split("(",QString::SkipEmptyParts);
        if(stringList.size() < minValidLineLength)
            continue;
        const auto firstDriver = stringList.first();
        const auto secondDriver = stringList.at(stringList.size()-2).split(incLineSplitter,
                                    QString::SkipEmptyParts).last();
        if(!incidentData.contains(qMakePair(secondDriver,firstDriver)))
            incidentData.push_back(DriverPair(firstDriver,secondDriver));
    }
    if(incidentData.isEmpty())
        qDebug()<<"Incidents: no incs were found";
    for(const auto& i : incidentData)
    {
        qDebug()<< "collision between " << i.first << " and " << i.second <<endl;
    }
    return incidentData;
}


QVector<DriverInfo> Parser::processDrivers(QXmlStreamReader& xml,const QVector<QString>& seqData) ////////TODO: fix
{
    QVector<DriverInfo> drivers;
    const QString XMLName = "Driver";
    const QString XMLEndName = "RaceResults";
    while(!hasError && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement &&
                          xml.name() == XMLEndName))
    {
        findXMLElement(xml,XMLName,XMLEndName,true);
        if(xml.name() != XMLName)
            break;
        DriverInfo currentDriver;
        for(const auto i : seqData)
        {
            if(hasError)
                break;
            findXMLElement(xml,i,XMLName);
            //if(xml.name() == "Lap")/////todo
            //    currentDriver.lapTimes = processDriverLaps(xml);
            xml.readNext();
            currentDriver.SeqElems.insert(i,xml.text().toString());
        }
        drivers.push_back(currentDriver);
    }
    if(hasError)
        return QVector<DriverInfo>{};
    return drivers;
}

QVector<QPair<int,double>> Parser::processDriverLaps(QXmlStreamReader& xml) const/////////TODO: fix
{
    const QString XMLName = "Lap";
    const QString notTimedLapText ="--.----";
    const QString XMLStartName = "BestLapTime";
    QVector<QPair<int,double>> LapTimes;
    QXmlStreamReader::TokenType token = xml.tokenType();
    while(!xml.hasError() && token != QXmlStreamReader::TokenType::StartElement &&
          xml.name() != XMLStartName)////////////if read error break with flushing Lap times
    {
       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == XMLName)
       {
            int LapNumber = 1;
            xml.readNext();
            if(xml.text() != notTimedLapText)
            {
                LapTimes.push_back(QPair<int,double>(LapNumber,xml.text().toDouble()));
            }
            else
            {
                LapTimes.push_back(QPair<int,double>(LapNumber,0.));
            }
            LapNumber++;
       }
    }
    //debug
    for(auto lap : LapTimes)
        qDebug()<< "Lap  num " << lap.first << "time " << lap.second << endl;
    return LapTimes;
}
