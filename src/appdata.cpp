#include "appdata.h"
#include <dbhelper.h>

UserData::UserData() { init(false); }

void UserData::init(bool isReset)
{
  try
  {
    dbObj.transactionStartLock();
    if (isReset)
    {
      dbObj.resetDB();
      updateSeasons();
      addSeason("Season 1");// adding default value
    } else
    {
      if (!dbObj.isValidDriver())
        throw std::runtime_error(
          QString("DataBase init error: no valid driver").toStdString());
      const bool isThereDB = dbObj.initDB();
      updateSeasons();
      if (!isThereDB) addSeason("Season 1");// adding default value
    }
    dbObj.transactionCommitUnlock();
  } catch (std::exception &e)
  {
    dbObj.transactionRollbackUnlock();
    throw std::runtime_error(e.what());
  }
}

SeasonData UserData::addSeason(const QString &name)
{
  dbObj.lock();
  int id = dbObj.addNewSeason(name);
  dbObj.unlock();
  SeasonData newSeason{ id, name };
  seasons.push_back(newSeason);
  return newSeason;
}

void UserData::updateSeasons()
{
  if (!seasons.isEmpty()) seasons.clear();
  dbObj.lock();
  auto seasonsTable = dbObj.getData(DBTableNames::Seasons);
  dbObj.unlock();
  for (const auto &i : seasonsTable)
  {
    SeasonData newSData;
    newSData.id = i.at(0).toInt();
    newSData.name = i.at(1).toString();
    seasons.push_back(newSData);
  }
}
