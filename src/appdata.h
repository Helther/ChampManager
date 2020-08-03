#ifndef APPDATA_H
#define APPDATA_H

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

  void init();
  SeasonData addSeason(const QString &name);
  void updateSeasons();
  ///SeasonData getCurrentSeason() { return currentSeason; }
  QVector<SeasonData> getSeasons() const { return seasons; }
  ///void setCurrentSeason(SeasonData season) { currentSeason = season; }

private:
  ///SeasonData currentSeason;
  QVector<SeasonData> seasons;
};

#endif// APPDATA_H
