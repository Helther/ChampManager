#ifndef DBHELPER_H
#define DBHELPER_H
#include <QtSql>
#include <parserConsts.h>

class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

  /* no copying*/
  DBHelper(const DBHelper &);
  DBHelper operator=(const DBHelper &);

  QSqlError initDB();

  QSqlError insertNewResults(const RaceLogInfo &inResults);

private:
  bool initConnection();
  QSqlError initResultsTables();

  QSqlDatabase dbConn;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
};

#endif// DBHELPER_H
