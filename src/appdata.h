#pragma once

#include <QtCore>


struct SeasonData
{
  int id;
  QString name;
  /// todo add other info like teams and drivers lists
};
Q_DECLARE_METATYPE(SeasonData)


//===========================User data============================//
// holds all meta data (currently only seasons info)
//======
class UserData
{
public:
  UserData();
  // inititalizes or resets DB and local data
  void init(bool isReset);
  // adds season to the DB and local array
  SeasonData addSeason(const QString &name);
  // repopulates seasons array
  void updateSeasons();
  ///SeasonData getCurrentSeason() { return currentSeason; }
  inline QVector<SeasonData> getSeasons() const { return seasons; }
  ///void setCurrentSeason(SeasonData season) { currentSeason = season; }

private:
  ///SeasonData currentSeason;
  QVector<SeasonData> seasons;
};

