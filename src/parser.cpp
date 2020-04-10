#include <parser.h>
#include "QXmlStreamReader"
//#ifdef QT_DEBUG
//#endif
Parser::Parser(const QString &FileName) : fileName(FileName)
{
    QFile file(FileName);
    if(openFile(file,QIODevice::ReadWrite))
    {
        auto buffer = file.readAll();
        fileData = QString(buffer).toUtf8();
        fileType = readFileType();
    }
}
Parser::~Parser()
{
    QFile file(fileName);
    if(file.isOpen())
        file.close();
}

FileType Parser::readFileType()////////////////////add precompiled string values for types and such
{
    const QString Racetype = "Race";
    const QString Qualtype = "Qualify";
    const QString Practtype = "Practice";
    const auto tokenName = "FixedUpgrades";
    //find log type token
    QXmlStreamReader xml(fileData);

    //is file xml file
    if(!xml.readNextStartElement())
    {
        qDebug()<<"file not xml\n";
        return readModFileType();
    }
    findXMLElement(xml,tokenName);
    xml.readNext();
    xml.readNext();
    xml.readNext();
    xml.readNext();//x4 to get to next token
    //xml error handling
    if(xml.hasError())
    {
        qDebug() << "XML error: " << xml.errorString() << '\n';
        return FileType::Error;
    }
    //return the correct type of the log
    if(xml.name() == Racetype)
    {
        qDebug() << "Race\n";
        return FileType::RaceLog;
    }
    if(xml.name() == Qualtype)
    {
        qDebug() << "Quali\n";
        return FileType::QualiLog;
    }
    if(xml.name().contains(Practtype))
    {
        qDebug() << "Practice\n";
        return FileType::PracticeLog;
    }
    qDebug() << "No valid log type was found\n";
    return FileType::Error;
}

FileType Parser::readModFileType() const
{
    const QString rcdFile = "rcd";
    const QString hdvFile = "HDV";
    const QString vehFile = "veh";
    QFile file(fileName);
    auto fileinfo = file.fileName().split(".",QString::KeepEmptyParts);
    const QString fileExt = fileinfo.at(fileinfo.size()-1);
    if(fileExt == rcdFile)
    {
        qDebug() << "rcd\n";
        return FileType::RCD;
    }
    if(fileExt == hdvFile)
    {
        qDebug() << "hdv\n";
        return FileType::HDV;
    }
    if(fileExt == vehFile)
    {
        qDebug() << "veh\n";
        return FileType::VEH;
    }
    return FileType::Error;
}

const QString Parser::findXMLElement(QXmlStreamReader& xml, const QString& elemName)
{
    QXmlStreamReader::TokenType token;
    QString result;
    while(!xml.atEnd() && !xml.hasError())
    {
        token = xml.readNext();
        if(token == QXmlStreamReader::TokenType::StartElement && xml.name() == elemName)
        {
            result = xml.text().toString();
            break;
        }
    }
    if(xml.hasError())
        raiseError(xml.errorString());
    if(xml.atEnd())
        raiseError("xml reader couldn't find the element: " + elemName);
    return result;
}

RaceLogInfo Parser::readPractQualiLog() const
{
    QXmlStreamReader xml(fileData);
    RaceLogInfo result;
    result.incidents = processIncidents(xml);
    return result;
}

RaceLogInfo Parser::readRaceLog() const
{
    QXmlStreamReader xml(fileData);
    RaceLogInfo result;
    result.incidents = processIncidents(xml);
    return result;
}

bool Parser::openFile(QFile& file,const QIODevice::OpenMode& mode) const
{
    if(file.exists())
    {
        if(!file.isOpen())
        {
            if(file.open(mode) | QIODevice::Text)
                return true;
            qDebug() << "Can't open file\n";
        }
        qDebug() << "Is already open!\n";
            return true;
    }
    qDebug()<< "File doesn't exist\n";
    return false;
}

bool Parser::backupFile(const QString &filePath,
                        const QString &backupPath,
                        const QString &backupName)
{
    QFile file(filePath);
    QFile bFile(backupPath);
    //set backup file new name
    auto fileinfo = file.fileName().split(".",QString::KeepEmptyParts);
    const QString fileExt = fileinfo.at(fileinfo.size()-1);
    fileinfo = file.fileName().split("/",QString::KeepEmptyParts);
    fileinfo = fileinfo.at(fileinfo.size()-1).split(".",QString::KeepEmptyParts);
    const QString fileName = fileinfo.at(0);
    //if no backup name was given set to date
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

bool Parser::readFileContent() const/////////finish
{
    ///after reading call static methods for writing to db
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
        default:
            return false;
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
        }
        else
        {
            qDebug()<<"raiseError: error msg is empty";
        }
    }
    else
        qDebug()<<"there is already an error: "<<errorMessage<<'\n';
}

QVector<DriverPair> Parser::processIncidents(QXmlStreamReader& xml) const/////////////fix
{
    const QString XMLIncName = "Incident";
    const QString XMLEndName = "Stream";
    QXmlStreamReader::TokenType token;
    QVector<QString> incidents;
    //parse and save incident strings
    QString prevDriverName;
    do
    {
    token = xml.readNext();
    if(token == QXmlStreamReader::TokenType::StartElement &&
            xml.name() == XMLIncName)
    {
        xml.readNext();
        const auto currentDriverName = xml.text().split("(",QString::SkipEmptyParts).
                first().toString();
        if(prevDriverName != currentDriverName)
            incidents.push_back(xml.text().toString());
        prevDriverName = currentDriverName;
    }
    }
    while(!(token == QXmlStreamReader::TokenType::EndElement &&
         xml.name() == XMLEndName));//////////////fix infinite loop
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

    for(const auto& i : incidentData)
    {
        qDebug()<< "collision between " << i.first << " and " << i.second <<endl;
    }
    return incidentData;
}


QVector<DriverInfo> Parser::processDrivers(QXmlStreamReader& xml) const////////fix
{
    QVector<DriverInfo> drivers;
    const QString XMLName = "Driver";
    const QString XMLEndName = "RaceResults";
    QXmlStreamReader::TokenType token;
    while(!xml.hasError() && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement &&
                                xml.name() == XMLEndName))
    {
        token = xml.readNext();
        if(token == QXmlStreamReader::TokenType::StartElement &&
                xml.name() == XMLName)
        {
            token = xml.readNext();
            DriverInfo driver;
            while(!xml.hasError() && token != QXmlStreamReader::TokenType::EndElement &&
                  xml.name() !=XMLName)/////infinite loop fix condition
            {
                //Parsing Laps
                //parse seq elems
                driver.lapTimes = processDriverLaps(xml);
            }
            drivers.append(driver);
        }
    }
    return drivers;
}

QVector<QPair<int,double>> Parser::processDriverLaps(QXmlStreamReader& xml) const/////////fix
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
