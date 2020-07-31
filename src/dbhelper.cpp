#include "dbhelper.h"

inline constexpr auto CREATE_SEASON = R"(create table seasons
                                      (season_id INTEGER not null,
                                      name TEXT,
                                      constraint k_season_id primary key (season_id)))";
inline constexpr auto CREATE_RACES = R"(create table races
                                     (race_id INTEGER not null,
                                     seasons_fid INTEGER not null,
                                     constraint k_race_id primary key (race_id),
                                     constraint k_seasons_fid foreign key (seasons_fid)
                                         references seasons(season_id)
                                            ON DELETE CASCADE))";
inline constexpr auto CREATE_RESULTS = R"(create table race_results
                                       (res_id INTEGER not null,
                                       session_fid INTEGER not null,
                                       Name TEXT,
                                       VehFile TEXT,
                                       VehName TEXT,
                                       CarType TEXT,
                                       CarClass TEXT,
                                       CarNumber TEXT,
                                       TeamName TEXT,
                                       GridPos INTEGER default null,
                                       Position INTEGER,
                                       ClassGridPos INTEGER default null,
                                       ClassPosition INTEGER,
                                       Lap TEXT,
                                       BestLapTime REAL,
                                       Pitstops INTEGER default null,
                                       FinishStatus TEXT default null,
                                       constraint k_res_id primary key (res_id),
                                       constraint k_session_fid foreign key (session_fid)
                                            references sessions (ses_id)
                                                ON DELETE CASCADE))";
inline constexpr auto CREATE_SESSIONS = R"(create table sessions
                                       (ses_id INTEGER not null,
                                       race_fid INTEGER not null,
                                       type TEXT,
                                       TrackVenue TEXT,
                                       RaceLaps INTEGER,
                                       constraint k_ses_id primary key (ses_id),
                                       constraint k_race_fid foreign key (race_fid)
                                            references races (race_id)
                                                ON DELETE CASCADE))";

DBHelper::DBHelper()
{
  dbConn = QSqlDatabase::database(connName);
  if (dbConn.isValid()) return;
  dbConn = QSqlDatabase::addDatabase(dbDriverName, connName);
  dbConn.setDatabaseName(dbName);
  if (!dbConn.open())
    throw std::runtime_error(QString("DataBase init error: ").toStdString()
                             + dbConn.lastError().text().toStdString());
}

DBHelper::~DBHelper() { dbConn.close(); }

bool DBHelper::initDB() const
{
  QFile db(dbName);
  if (!db.exists() || dbConn.tables().size() == 0)
  {
    auto resultsError = initResultsTables();
    checkSqlError("initDB error", resultsError);
    return false;
  }
  //table existence check
  const QVector<QString> tableNames{ DBTableNames::Races,
                                     DBTableNames::RaceRes,
                                     DBTableNames::Seasons,
                                     DBTableNames::Sessions };
  for (const auto &table : tableNames)
  {
    if (!dbConn.tables().contains(table))
      throw std::runtime_error(
        QString("Init DB error: table not found - ").toStdString()
        + table.toStdString());
  }
  return true;
}

void DBHelper::destroyDB() const
{
  const QString query = "drop table ";
  QSqlQuery q(dbConn);
  QSqlError destroyError;
  if (dbConn.tables().contains(DBTableNames::Seasons)
      && !q.exec(query + DBTableNames::Seasons))
    destroyError = q.lastError();
  if (dbConn.tables().contains(DBTableNames::Races)
      && !q.exec(query + DBTableNames::Races))
    destroyError = q.lastError();
  if (dbConn.tables().contains(DBTableNames::RaceRes)
      && !q.exec(query + DBTableNames::RaceRes))
    destroyError = q.lastError();
  if (dbConn.tables().contains(DBTableNames::Sessions)
      && !q.exec(query + DBTableNames::Sessions))
    destroyError = q.lastError();
  checkSqlError("destr db error", destroyError);
}

void DBHelper::addNewResults(const RaceLogInfo &inResults, int sessionId) const
{
  const QString queryColumns{ "insert into " + DBTableNames::RaceRes
                              + " ( session_fid" };
  const QString queryValues{ "values (?" };
  QSqlQuery q(dbConn);
  for (const auto &driver : inResults.drivers)
  {
    QString qC = queryColumns;
    QString qV = queryValues;
    for (const auto &i : driver.SeqElems)
    {
      qC.append(", " + i.first);
      qV.append(", ?");
    }
    qC.append(")");
    qV.append(")");
    q.prepare(qC + qV);
    q.addBindValue(QString::number(sessionId));
    for (const auto &i : driver.SeqElems) q.addBindValue(i.second);
    if (!q.exec()) checkSqlError("Insert New Race Res error", q.lastError());
  }
}

int DBHelper::addNewRace(int seasonId) const
{
  QSqlQuery q(dbConn);
  q.prepare("insert into " + DBTableNames::Races + R"((
    seasons_fid
    ) values (?))");
  q.addBindValue(seasonId);
  if (!q.exec()) checkSqlError("add new race error", q.lastError());
  return q.lastInsertId().toInt();
}

int DBHelper::addNewSeason(const QString &name) const
{
  QSqlQuery q(dbConn);
  QString query{ "insert into " };
  query.append(DBTableNames::Seasons);
  query.append(" (name) values (\"");
  query.append(name);
  query.append("\")");
  if (!q.exec(query)) checkSqlError("add new season error", q.lastError());
  return q.lastInsertId().toInt();
}

void DBHelper::viewTable(const QString &tableName) const
{
  QSqlQuery q(dbConn);
  if (!q.exec("select * from " + tableName))
    checkSqlError("view table error", q.lastError());
  while (q.next())
  {
    for (int i = 0; i < q.record().count(); ++i)
    { qDebug() << q.record().fieldName(i) << q.value(i).toString() << " "; }
    qDebug() << '\n';
  }
}

QVector<QVector<QVariant>> DBHelper::getData(const QString &tableName) const
{
  QSqlQuery q(dbConn);
  QVector<QVector<QVariant>> result;
  if (!q.exec("select * from " + tableName))
    checkSqlError("get table data error", q.lastError());
  while (q.next())
  {
    QVector<QVariant> row;
    for (int i = 0; i < q.record().count(); ++i) row.push_back(q.value(i));
    result.push_back(row);
  }
  return result;
}

QSqlError DBHelper::initResultsTables() const
{
  QSqlQuery init(dbConn);
  if (!init.exec(CREATE_SEASON)) return init.lastError();
  if (!init.exec(CREATE_RACES)) return init.lastError();
  if (!init.exec(CREATE_RESULTS)) return init.lastError();
  if (!init.exec(CREATE_SESSIONS)) return init.lastError();
  return QSqlError();
}


int DBHelper::addNewSession(const QString &type,
                            int raceId,
                            const RaceLogInfo &sesData) const
{
  const auto trackName =
    std::find_if(sesData.SeqElems.begin(),
                 sesData.SeqElems.end(),
                 [](const auto &i) { return i.first == "TrackVenue"; });
  const auto lapsNum =
    std::find_if(sesData.SeqElems.begin(),
                 sesData.SeqElems.end(),
                 [](const auto &i) { return i.first == "RaceLaps"; });
  if (trackName == sesData.SeqElems.end() || lapsNum == sesData.SeqElems.end())
    throw std::runtime_error("add new session error: data incomplete");

  QSqlQuery q(dbConn);
  QString query{ "insert into " };
  query.append(DBTableNames::Sessions);
  query.append(" (type, TrackVenue, Racelaps, race_fid) values (?, ?, ?, ?)");
  q.prepare(query);
  q.addBindValue(type);
  q.addBindValue(trackName->second);
  q.addBindValue(lapsNum->second);
  q.addBindValue(raceId);
  if (!q.exec()) checkSqlError("add new session error", q.lastError());
  return q.lastInsertId().toInt();
}

void DBHelper::delSeasonData(int ses_id) const
{
  delEntryFromTable(DBTableNames::Seasons, "season_id", ses_id);
}

void DBHelper::checkSessionsValidity(const QVector<int> &ids) const
{
  SessionData SData;
  bool first = true;
  for (auto id : ids)
  {
    QSqlQuery q(dbConn);
    if (!q.exec("select TrackVenue, RaceLaps from " + DBTableNames::Sessions
                + " where ses_id = " + QString::number(id)))
      checkSqlError("session validity error", q.lastError());
    q.next();
    if (first)
    {
      SData = SessionData{ q.value(0).toString(), q.value(1).toString() };
      first = false;
    } else
    {
      SessionData curSData{ q.value(0).toString(), q.value(1).toString() };
      if (curSData != SData)
        throw std::runtime_error("sessions validity error: incompatible data");
      SData = curSData;
    }
  }
}

void DBHelper::delEntryFromTable(const QString &table,
                                 const QString &idCol,
                                 int id) const
{
  QSqlQuery q(dbConn);
  // exec this to activate Fkeys On actions
  if (!q.exec("PRAGMA foreign_keys=ON"))
    checkSqlError("delete table entry error", q.lastError());
  QString query{ "delete from " };
  query.append(table);
  query.append(" where " + idCol + " = " + QString::number(id));
  if (!q.exec(query)) checkSqlError("delete table entry error", q.lastError());
}

void DBHelper::checkSqlError(const QString &msg, const QSqlError &error) const
{
  if (error.type() != QSqlError::NoError)
    throw std::runtime_error(msg.toStdString() + " : "
                             + error.text().toStdString());
}
