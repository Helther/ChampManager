#ifndef DBHELPER_H
#define DBHELPER_H
#include <QtSql>
#include <parserConsts.h>

struct RaceInputData
{
  int Pid;
  int Qid;
  int Rid;
  int Seasonid;
  QString Track;
  int LapsNum;
};

class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

  /* no copying*/
  DBHelper(const DBHelper &) = delete;
  DBHelper operator=(const DBHelper &) = delete;

  void initDB();
  // called when file was read
  void insertNewResults(const RaceLogInfo &inResults);
  // called after all results were collected with ids as args
  void addNewRace(const RaceInputData &data);
  void addNewSeason(const QString &name);

private:
  bool initConnection();
  QSqlError initResultsTables();
  // throws if there was an sql error
  void checkSqlError(const QString &msg, const QSqlError &error);

  QSqlDatabase dbConn;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
};

#endif// DBHELPER_H
