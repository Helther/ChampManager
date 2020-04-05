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

bool Parser::readFileContent()/////////finish
{
    ///after reading call static methods for writing to db
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

QVector<DriverPair> Parser::Incidents(QXmlStreamReader& xml)/////////////fix
{
    const QString XMLIncName = "Incident";
    const QString XMLEndName = "Stream";
    QVector<DriverPair> incidentData;
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
        auto currentDriverName = xml.text().split("(",QString::SkipEmptyParts).
                first().toString();
        if(prevDriverName != currentDriverName)
            incidents.push_back(xml.text().toString());
        prevDriverName = currentDriverName;
    }
    }
    while(!(token == QXmlStreamReader::TokenType::EndElement &&
         xml.name() == XMLEndName));

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
    return incidentData;
}


QVector<DriverInfo> Parser::Drivers(QXmlStreamReader& xml)////////fix
{
    QVector<DriverInfo> drivers;
    const QString XMLName = "Driver";
    QXmlStreamReader::TokenType token = xml.tokenType();
    while(!xml.hasError() && !(xml.tokenType() == QXmlStreamReader::TokenType::EndElement &&
          xml.name() == "RaceResults"))
    {
        token = xml.readNext();
        if(token == QXmlStreamReader::TokenType::StartElement &&
                xml.name() == XMLName)
        {
            token = xml.readNext();
            DriverInfo driver;
            //infinite loop fix condition
            while(!xml.hasError() && token != QXmlStreamReader::TokenType::EndElement &&
                  xml.name() !=XMLName)
            {
                //Parsing Laps
                driver.lapTimes = DriverLaps(xml);
                //parse seq elems
            }
            drivers.append(driver);
        }
    }
    return drivers;
}

QVector<QPair<int,double>> Parser::DriverLaps(QXmlStreamReader& xml)/////////fix
{
    const QString XMLName = "Lap";
    QVector<QPair<int,double>> LapTimes;
    QXmlStreamReader::TokenType token = xml.tokenType();
    while(!xml.hasError() && token != QXmlStreamReader::TokenType::StartElement &&
          xml.name() != "BestLapTime")
    {
       int LapNumber = 1;
       token = xml.readNext();
       if(token == QXmlStreamReader::TokenType::StartElement &&
               xml.name() == XMLName)
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
