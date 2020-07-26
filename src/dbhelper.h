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

struct RaceInputData
{
  int Pid;
  int Qid;
  int Rid;
  int Seasonid;
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
  bool initDB();
  // deletes all tables in db, called if init has failed
  void destroyDB();
  // called when file was read
  void addNewResults(const RaceLogInfo &inResults, int sessionId);
  // called after all results were collected with ids as args
  int addNewRace(const RaceInputData &data);
  // inserts new entry into seasons table
  int addNewSeason(const QString &name);
  // inserts new entry into sessions table
  int addNewSession(const QString &type);

  void delEntryFromTable(const QString &table, const QString &idCol, int id);

  QVector<QVector<QVariant>> getData(const QString &tableName);

  ///debug
  void viewTable(const QString &tableName);

private:
  QSqlError initResultsTables();

  //==========================misc============================//
  // throws if there was an sql error
  void checkSqlError(const QString &msg, const QSqlError &error);
  //===========================data==========================//
  QSqlDatabase dbConn;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
};

#endif// DBHELPER_H
