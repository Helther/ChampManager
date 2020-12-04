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
                                         references seasons (season_id)
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
                                       TimeString TEXT,
                                       TrackCourse TEXT,
                                       RaceLaps INTEGER,
                                       constraint k_ses_id primary key (ses_id),
                                       constraint k_race_fid foreign key (race_fid)
                                            references races (race_id)
                                                ON DELETE CASCADE))";
inline constexpr auto CREATE_INCIDENTS = R"(create table incidents
                                        (inc_id INTEGER not null,
                                        session_fid INTEGER not null,
                                        first_driver TEXT,
                                        second_driver TEXT,
                                        constraint k_inc_id primary key (inc_id),
                                        constraint k_session_fid foreign key (session_fid)
                                            references sessions (ses_id)
                                                ON DELETE CASCADE))";
DBHelper::DBHelper() { initConnection(); }

DBHelper::~DBHelper()
{
  if (!wasAlreadyOpen && dbConn.isOpen()) dbConn.close();
}

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
  for (const auto &table : allTableNames)
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
  Perf p("destrdb");/// todo temp
  const QString query = "drop table ";
  QSqlQuery q(dbConn);
  QSqlError destroyError;
  for (const auto &table : allTableNames)
  {
    if (dbConn.tables().contains(table) && !q.exec(query + table))
    {
      destroyError = q.lastError();
      break;
    }
  }
  checkSqlError("destr db error", destroyError);
}

void DBHelper::addNewResults(const RaceLogInfo &inResults, int sessionId) const
{// caller must start transaction
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

    addIncidents(inResults.incidents, sessionId);
  }
}

void DBHelper::addIncidents(const QVector<StringPair> &incindents,
                            int ses_id) const
{// caller must start a transaction
  QSqlQuery q(dbConn);
  for (const auto &inc : incindents)
  {
    if (!q.exec(QString("insert into %1 (session_fid, first_driver, "
                        "second_driver) values( %2, "
                        "\"%3\", \"%4\")")
                  .arg(DBTableNames::Incidents)
                  .arg(ses_id)
                  .arg(inc.first)
                  .arg(inc.second)))
      checkSqlError("Insert New incident error", q.lastError());
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
  auto query = QString("insert into %1 (name) values ( \"%2\" )")
                 .arg(DBTableNames::Seasons)
                 .arg(name);
  if (!q.exec(query)) checkSqlError("add new season error", q.lastError());
  return q.lastInsertId().toInt();
}
#ifdef QT_DEBUG
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
#endif

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

QVector<RaceData> DBHelper::getRaceData(int seasonId)
{
  QSqlQuery q(dbConn);
  if (!q.exec(QString("select race_id from %1 where seasons_fid = %2")
                .arg(DBTableNames::Races)
                .arg(seasonId)))
    checkSqlError("get race data, race list error", q.lastError());
  QVector<RaceData> retData;
  while (q.next())
  {
    retData.push_back(RaceData());
    retData.last().raceId = q.value(0).toInt();
  }
  for (auto &race : retData)
  {
    if (!q.exec(
          QString("select ses_id, type, TimeString, TrackCourse, RaceLaps "
                  "from %1 where race_fid = %2")
            .arg(DBTableNames::Sessions)
            .arg(race.raceId)))
      checkSqlError("get race data, session list error", q.lastError());
    while (q.next())
    {
      race.sessions.push_back({ q.value(0).toInt(), q.value(1).toString() });
    }
    q.seek(0);
    race.date = q.value(2).toString();
    race.track = q.value(3).toString();
    race.laps = q.value(4).toInt();
  }
  return retData;
}

QSqlError DBHelper::initResultsTables() const
{// caller must start transaction
  Perf p("initdb");/// todo temp
  QSqlQuery init(dbConn);
  if (!init.exec(CREATE_SEASON)) return init.lastError();/// todo refactor
  if (!init.exec(CREATE_RACES)) return init.lastError();
  if (!init.exec(CREATE_RESULTS)) return init.lastError();
  if (!init.exec(CREATE_SESSIONS)) return init.lastError();
  if (!init.exec(CREATE_INCIDENTS)) return init.lastError();
  return QSqlError();
}

void DBHelper::initConnection()
{
  dbConn = QSqlDatabase::database(connName);
  if (dbConn.isOpen())
  {
    wasAlreadyOpen = true;
    return;
  }
  dbConn = QSqlDatabase::addDatabase(dbDriverName, connName);
  dbConn.setDatabaseName(dbName);
  dbConn.open();
}

int DBHelper::addNewSession(const QString &type,
                            int raceId,
                            const RaceLogInfo &sesData) const
{
  const auto time =
    std::find_if(sesData.SeqElems.begin(),
                 sesData.SeqElems.end(),
                 [](const auto &i) { return i.first == "TimeString"; });
  const auto trackName =
    std::find_if(sesData.SeqElems.begin(),
                 sesData.SeqElems.end(),
                 [](const auto &i) { return i.first == "TrackCourse"; });
  const auto lapsNum =
    std::find_if(sesData.SeqElems.begin(),
                 sesData.SeqElems.end(),
                 [](const auto &i) { return i.first == "RaceLaps"; });
  if (trackName == sesData.SeqElems.end() || lapsNum == sesData.SeqElems.end()
      || time == sesData.SeqElems.end())
    throw std::runtime_error("add new session error: data incomplete");

  QSqlQuery q(dbConn);
  QString query{ QString("insert into %1 (type, TimeString, TrackCourse, "
                         "Racelaps, race_fid) values (?, ?, ?, ?, ?)")
                   .arg(DBTableNames::Sessions) };
  q.prepare(query);
  q.addBindValue(type);
  q.addBindValue(time->second);
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

void DBHelper::delSeasonRaces(int ses_id) const
{
  QSqlQuery q(dbConn);
  if (!q.exec(QString("select race_id from %1 where seasons_fid = %2")
                .arg(DBTableNames::Races)
                .arg(ses_id)))
    checkSqlError("all races in season deletion error", q.lastError());
  QVector<int> raceids;
  while (q.next()) raceids.push_back(q.value(0).toInt());
  for (const auto i : raceids)
    delEntryFromTable(DBTableNames::Races, "race_id", i);
}

void DBHelper::checkSessionsValidity(const QVector<int> &ids) const
{// caller must start transaction
  SessionData SData;
  bool first = true;
  for (auto id : ids)
  {
    QSqlQuery q(dbConn);
    auto query =
      QString("select TrackCourse, RaceLaps from %1 where ses_id = %2")
        .arg(DBTableNames::Sessions)
        .arg(QString::number(id));
    if (!q.exec(query)) checkSqlError("session validity error", q.lastError());
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
                                 const QString &idColName,
                                 int id) const
{// caller must start transaction
  QSqlQuery q(dbConn);
  // exec this to activate Fkeys On actions
  if (!q.exec("PRAGMA foreign_keys=ON"))
    checkSqlError("delete table entry error", q.lastError());
  QString query{ QString("delete from %1 where %2 = %3")
                   .arg(table)
                   .arg(idColName)
                   .arg(QString::number(id)) };
  if (!q.exec(query)) checkSqlError("delete table entry error", q.lastError());
}

void DBHelper::resetDB()
{// caller must start transaction
  Perf p("reset db");/// todo temp
  QSqlQuery q(dbConn);
  for (const auto &table : allTableNames)
    if (!q.exec(QString("delete from %1").arg(table)))
      checkSqlError("reset db error", q.lastError());
}

void DBHelper::checkSqlError(const QString &msg, const QSqlError &error) const
{
  if (error.type() != QSqlError::NoError)
    throw std::runtime_error(msg.toStdString() + " : "
                             + error.text().toStdString());
}
