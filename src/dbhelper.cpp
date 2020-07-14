#include "dbhelper.h"
#include <parserConsts.h>


DBHelper::DBHelper()
{
  dbConn = QSqlDatabase::addDatabase(dbDriverName, dbPath + connName);
  qDebug() << "constr start" << dbConn.isOpen();
  if (!initConnection())
    throw std::runtime_error(QString("DataBase init error: ").toStdString()
                             + dbConn.lastError().text().toStdString());
}

DBHelper::~DBHelper()
{
  qDebug() << "destr start" << dbConn.isOpen();
  dbConn.close();
  // QSqlDatabase::removeDatabase(connName);
  qDebug() << "destr start" << dbConn.isOpen();
}

bool DBHelper::initConnection()
{
  dbConn.setDatabaseName(dbName);
  return dbConn.isValid() && dbConn.open();
}
