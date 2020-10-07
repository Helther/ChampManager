#include "appdata.h"
#include <dbhelper.h>

UserData::UserData() { init(false); }

void UserData::init(bool isReset)
{
  DBHelper db;
  try
  {
    db.transactionStart();//------------ transact start
    if (isReset)
    {
      db.resetDB();
      updateSeasons();
      addSeason("Season 1");// adding default value
    } else
    {
      const bool isThereDB = db.initDB();
      updateSeasons();
      if (!isThereDB) addSeason("Season 1");// adding default value
    }
    db.transactionCommit();//------------ transact commit
  } catch (std::exception &e)
  {
    db.transactionRollback();
    throw std::runtime_error(e.what());
  }
}

SeasonData UserData::addSeason(const QString &name)
{
  DBHelper db;
  int id = db.addNewSeason(name);
  SeasonData newSeason{ id, name };
  seasons.push_back(newSeason);
  return newSeason;
}

void UserData::updateSeasons()
{
  DBHelper db;
  if (!seasons.isEmpty()) seasons.clear();
  auto seasonsTable = db.getData(DBTableNames::Seasons);
  for (const auto &i : seasonsTable)
  {
    SeasonData newSData;
    newSData.id = i.at(0).toInt();
    newSData.name = i.at(1).toString();
    seasons.push_back(newSData);
  }
}
