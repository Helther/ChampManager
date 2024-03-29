#pragma once
#include <QtSql>
#include <parserConsts.h>


namespace DBTableNames {
inline const QString Races = "races";
inline const QString RaceRes = "race_results";
inline const QString Seasons = "seasons";
inline const QString Sessions = "sessions";
inline const QString Incidents = "incidents";
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
  QVector<QPair<int, QString>> sessions;  // first - session id, second - type
};

//======================== DataBase interface =====================//
// class that holds db connection object and perfoms all procedures
// that involve db write or read
// multitheading sync is done by blocking through transaction mechanism
//===========
class DBHelper
{
public:
  DBHelper();
  ~DBHelper();

  // called when first time launch the app returns false if can't find the db
  bool initDB() const;
  // deletes all tables in db, called if init has failed
  void destroyDB() const;
  bool isValidDriver() const { return dbConn.isValid(); }
  // called when file was read
  void addNewResults(const RaceLogInfo &inResults, int sessionId) const;
  // called from addNewResults
  void addIncidents(const QVector<StringPair> &incindents, int ses_id) const;
  // called after all results were collected with ids as args
  [[nodiscard]] int addNewRace(int seasonId) const;
  // inserts new entry into seasons table
  [[nodiscard]] int addNewSeason(const QString &name) const;
  // inserts new entry into sessions table
  [[nodiscard]] int
    addNewSession(const QString &type, int raceId, const RaceLogInfo &sesData) const;
  // deletes tables entries in races and rec_res and sessions tables
  // corresponding to a given season
  void delSeasonData(int ses_id) const;
  // deletes all races data in a given season
  void delSeasonRaces(int ses_id) const;
  // checks whether sessions a part of the same race
  void checkSessionsValidity(const QVector<int> &ids) const;
  // deletes row with a given id in a table, where primary column name is idCol
  void delEntryFromTable(const QString &table, const QString &idColName, int id) const;
  // delets all entries from all tables
  void resetDB();
  // select from all table columns
  [[nodiscard]] QVector<QVector<QVariant>> getData(const QString &tableName) const;
  // return data for building results item model
  [[nodiscard]] QVector<RaceData> getRaceData(int seasonId);

  QSqlDatabase getDBConnection() { return dbConn; }

  void transactionStartLock()
  {
    lock();
    if (!dbConn.transaction())
      throw std::runtime_error("db transaction start error");
  }
  void transactionCommitUnlock()
  {
    unlock();
    if (!dbConn.commit())
      throw std::runtime_error("db transaction commit error");
  }
  void transactionRollbackUnlock()
  {
    unlock();
    if (!dbConn.rollback())
      throw std::runtime_error("db transaction rollback error");
  }

  void lock() { mutex.lock(); }
  void unlock() { mutex.unlock(); }
#ifdef QT_DEBUG
  void viewTable(const QString &tableName) const;
#endif
private:
  // creates tables in dataBase
  QSqlError initResultsTables() const;
  void initConnection();

  //==========================misc============================//
  // throws if there was an sql error
  void checkSqlError(const QString &msg, const QSqlError &error) const;
  //===========================data==========================//
  QRecursiveMutex mutex;
  QSqlDatabase dbConn;
  bool wasAlreadyOpen = false;
  const QString dbDriverName = "QSQLITE";
  QString connName = "dbConn";
  const QString dbName = "CMM.db3";
  const QString dbPath = "./";  /// todo temp
  const QVector<QString> allTableNames{ DBTableNames::Races,
                                        DBTableNames::RaceRes,
                                        DBTableNames::Seasons,
                                        DBTableNames::Sessions,
                                        DBTableNames::Incidents };
};

// global object for blocking db access
static DBHelper dbObj;

