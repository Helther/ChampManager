#include "parser.h"
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
        file.close();
    }
}

FileType Parser::readFileType()
{
    //find log type token
    QXmlStreamReader xml(fileData);
    QXmlStreamReader::TokenType token;
    while(!xml.atEnd() && !xml.hasError())
    {
       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "FixedUpgrades")
       {
           qDebug() << "found FixedUpgrades line\n";
           do
           {
            token = xml.readNext();
           }
           while(token != QXmlStreamReader::TokenType::StartElement &&
                 !xml.atEnd() && !xml.hasError());
           break;
       }
    }
    //error handling
    if (xml.hasError())
    {
        qDebug() << "XML error: " << xml.errorString() << '\n';
        return FileType::Error;
        ///add get non xml file types
    }
    //return the correct type of the log
    if(xml.name() == "Race")
    {
        qDebug() << "Race\n";
        return FileType::RaceLog;
    }
    if(xml.name() == "Qualify")
    {
        qDebug() << "Quali\n";
        return FileType::QualiLog;
    }
    if(xml.name().contains("Practice"))
    {
        qDebug() << "Practice\n";
        return FileType::PracticeLog;
    }
    return FileType::Error;
}

bool Parser::openFile(QFile& file,const QIODevice::OpenMode& mode)
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
                return false;
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
/*
template<>
void Parser::pParams<QVector<QString>>(QXmlStreamReader &xml, QVector<QString>& data,const QVector<QString>& elemNames)
{
    if(data.size() != elemNames.size())
    {
        qDebug()<< "pParams: bad arguments format\n";
        return;
    }
    for(auto i = 0; i< elemNames.size();++i)
    {
        if(xml.tokenType() == QXmlStreamReader::TokenType::StartElement &&
                xml.name().toString() == elemNames.at(i))
        {
            data[i] = xml.text().toString();
        }
        xml.readNext();
    }
}
*/

bool Parser::readFileContent()
{
    SeqDataStruct elements;
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
            break;
        }
        case FileType::QualiLog :
        case FileType::PracticeLog :
        {

            break;
        }
        default:
            return false;
    }

    //catch excepts
    return false;
}

void Parser::MainInfo(LogInfo &log)
{
    QXmlStreamReader xml(fileData);
    QXmlStreamReader::TokenType token;
    while(!xml.atEnd() && !xml.hasError())
    {

       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "PlayerFile")
       {
           xml.readNext();
           log.playerName = xml.text().toString();
           continue;
       }
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "Mod")
       {
           xml.readNext();
           log.modName = xml.text().toString();
           continue;
       }
       if(xml.tokenType() == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "TrackVenue")
       {
           xml.readNext();
           log.trackName = xml.text().toString();
           if(fileType == FileType::RaceLog)
           {
              while(!(xml.tokenType() == QXmlStreamReader::TokenType::StartElement &&
                    xml.name() == "Laps"))
              {
                xml.readNext();
              }
                xml.readNext();
                log.numberOfLaps = xml.text().toInt();
           }
           break;
       }
    }
    //output debug
    qDebug()<< "PlayerFile " << log.playerName << endl << "Mod " << log.modName << endl
            << "TrackVenue" << log.trackName << endl << "Num of Laps" << log.numberOfLaps
            << endl;
}

QVector<DriverPair> Parser::Incidents(QXmlStreamReader& xml)
{
    QVector<DriverPair> incidentData;
    QXmlStreamReader xml(fileData);
    QXmlStreamReader::TokenType token;
    QVector<QString> incidents;
    //parse and save incident strings
    while(!xml.atEnd() && !xml.hasError())
    {
       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "Stream")
       {
           qDebug() << "found " << xml.name() <<endl;
           QString prevDriverName;
           do
           {
            token = xml.readNext();
            if(token == QXmlStreamReader::TokenType::StartElement &&
                    xml.name() == "Incident")
            {
                xml.readNext();
                auto currentDriverName = xml.text().split("(",QString::SkipEmptyParts).
                        first().toString();
                if(prevDriverName != currentDriverName)
                    incidents.push_back(xml.text().toString());
                prevDriverName = currentDriverName;
            }
           }
           while(!(token == QXmlStreamReader::TokenType::EndElement &&
                 xml.name() == "Stream"));
           break;
       }
    }
    //ignore equal driver combinations
    QString currentString;
    for(auto i = incidents.cbegin(); i < incidents.cend();++i)
    {
            currentString = *i;
            auto stringList = currentString.split("(",QString::SkipEmptyParts);
            if(stringList.size() < 4)
                continue;
            auto firstDriver = stringList.first();
            auto secondDriver = stringList.at(stringList.size()-2).split("vehicle ",
                                               QString::SkipEmptyParts).last();
            bool isEqual = false;
            for(const auto& j : incidentData)
            {
                if(j.first == secondDriver && j.second == firstDriver)
                {
                    isEqual = true;
                    break;
                }
            }
            if(!isEqual)
                incidentData.push_back(DriverPair(firstDriver,secondDriver));
    }
    //output
    for(const auto& i : incidentData)
    {
        qDebug()<< "collision between " << i.first << " and " << i.second <<endl;
    }
    xml.incidents = incidentData;
}


void Parser::DriverMain(LogInfo &log)
{
    QXmlStreamReader xml(fileData);
    QXmlStreamReader::TokenType token = xml.tokenType();
    while(!xml.hasError() && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement &&
          xml.name() == "RaceResults"))
    {
        token = xml.readNext();
        if(token == QXmlStreamReader::TokenType::StartElement &&
                xml.name() == "Driver")
        {
            token = xml.readNext();
            DriverInfo driver;
            //infinite loop fix condition
            while(!xml.hasError() && token != QXmlStreamReader::TokenType::EndElement &&
                  xml.name() != "Driver")
            {
                //Parsing Laps
                driver.lapTimes = DriverLaps();
                double bestLap{};
                for(auto lap : driver.lapTimes)
                {
                    if(lap.second > bestLap)
                        bestLap = lap.second;
                }
                driver.bestLapTime = bestLap;
            }
            log.drivers.push_back(driver);
        }
    }
}

QList<QPair<int,double>> Parser::DriverLaps(QXmlStreamReader& xml)
{
    QList<QPair<int,double>> LapTimes;
    QXmlStreamReader xml(fileData);
    QXmlStreamReader::TokenType token = xml.tokenType();
    while(!xml.hasError() && token != QXmlStreamReader::TokenType::StartElement &&
          xml.name() != "BestLapTime")
    {
       int LapNumber = 1;
       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == "Lap")
       {
            xml.readNext();
            if(xml.text() != "--.----")
            {
                LapTimes.push_back(QPair<int,double>(LapNumber,xml.text().toDouble()));
            }
            else
            {
                LapTimes.push_back(QPair<int,double>(LapNumber,0.0));
            }
            LapNumber++;
       }
    }
    //debug
    for(auto lap : LapTimes)
        qDebug()<< "Lap  num " << lap.first << "time " << lap.second << endl;
    return LapTimes;
}
