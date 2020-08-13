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
// data necessary for sessions compatabilty check
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

// data of all sessions in a race needed for results display
struct RaceData
{
  int raceId;
  QString track;
  int laps;
  QString date;
  QVector<QPair<int, QString>> sessions;// first - session id, second - type
};

//======================== DataBase interface =====================//
// class that holds db connection object and perfoms all procedures
// that involve db write or read
//===========
/// todo add incidents save into db
class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

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
  // checks whether sessions a part of the same race
  void checkSessionsValidity(const QVector<int> &ids) const;
  // deletes row with a given id in a table, where primary column name is idCol
  void delEntryFromTable(const QString &table,
                         const QString &idColName,
                         int id) const;
  // delets all entries from all tables
  void resetDB();
  // select from all table columns
  QVector<QVector<QVariant>> getData(const QString &tableName) const;
  // return data for building results item model
  QVector<RaceData> getRaceData(int seasonId);

  inline QSqlDatabase getDBConnection() { return dbConn; }

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
#ifdef QT_DEBUG
  ///debug
  void viewTable(const QString &tableName) const;
#endif
private:
  // creates tables in dataBase
  QSqlError initResultsTables() const;


  //==========================misc============================//
  // throws if there was an sql error
  void checkSqlError(const QString &msg, const QSqlError &error) const;
  //===========================data==========================//
  QSqlDatabase dbConn;
  bool wasAlreadyOpen = false;
  const QString dbDriverName = "QSQLITE";
  const QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";/// todo temp
  const QVector<QString> allTableNames{ DBTableNames::Races,
                                        DBTableNames::RaceRes,
                                        DBTableNames::Seasons,
                                        DBTableNames::Sessions };
};

#endif// DBHELPER_H
