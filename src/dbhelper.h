#ifndef DBHELPER_H
#define DBHELPER_H
#include <QtCore>
#include <QtSql>
// class QSqlDatabase;

class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

  /* no copying*/
  DBHelper(const DBHelper &);
  DBHelper &operator=(const DBHelper &);

  QSqlDatabase getConnection() { return dbConn; }

private:
  bool initConnection();

  QSqlDatabase dbConn;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
};

#endif// DBHELPER_H
