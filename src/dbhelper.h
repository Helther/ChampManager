#ifndef DBHELPER_H
#define DBHELPER_H
#include <QtSql>
#include <parserConsts.h>

namespace DBTableNames {
inline const QString Races = "races";
inline const QString RaceRes = "race_results";
inline const QString Seasons = "seasons";
inline const QString Sessions = "sessions";
}

struct SessionData
{
  QString venue;
  QString laps;
  bool operator==(const SessionData &rhs) const
  {
    return venue == rhs.venue && laps == rhs.laps;
  }
  bool operator!=(const SessionData &rhs) const { return !(*this == rhs); }
};
/// todo add incidents save into db
class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

  /* no copying*/
  DBHelper(const DBHelper &) = delete;
  DBHelper operator=(const DBHelper &) = delete;

  // called when first time launch the app returns false if can't find the db
  bool initDB() const;
  // deletes all tables in db, called if init has failed
  void destroyDB() const;
  // called when file was read
  void addNewResults(const RaceLogInfo &inResults, int sessionId) const;
  // called after all results were collected with ids as args
  int addNewRace(int seasonId) const;
  // inserts new entry into seasons table
  int addNewSeason(const QString &name) const;
  // inserts new entry into sessions table
  int addNewSession(const QString &type,
                    int raceId,
                    const RaceLogInfo &sesData) const;
  // deletes tables entries in races and rec_res and sessions tables cirresponding to a given season
  void delSeasonData(int ses_id) const;

  void checkSessionsValidity(const QVector<int> &ids) const;

  void
    delEntryFromTable(const QString &table, const QString &idCol, int id) const;

  QVector<QVector<QVariant>> getData(const QString &tableName) const;

  void transactionStart()
  {
    if (!dbConn.transaction())
      throw std::runtime_error("db transaction start error");
  }
  void transactionCommit()
  {
    if (!dbConn.commit())
      throw std::runtime_error("db transaction commit error");
  }
  void transactionRollback()
  {
    if (!dbConn.rollback())
      throw std::runtime_error("db transaction rollback error");
  }

  ///debug
  void viewTable(const QString &tableName) const;

private:
  QSqlError initResultsTables() const;
  // checks if given sessions are from the same race and returns data needed for addRace


  //==========================misc============================//
  // throws if there was an sql error
  void checkSqlError(const QString &msg, const QSqlError &error) const;
  //===========================data==========================//
  QSqlDatabase dbConn;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
};

#endif// DBHELPER_H
