#include "dbhelper.h"

inline constexpr auto CREATE_SEASON = R"(create table seasons
                                      (season_id INTEGER not null,
                                      name TEXT,
                                      constraint k_season_id primary key (season_id)))";
inline constexpr auto CREATE_RACES = R"(create table races
                                     (race_id INTEGER not null,
                                     pract_fid INTEGER not null,
                                     quali_fid INTEGER not null,
                                     race_fid INTEGER not null,
                                     seasons_fid INTEGER not null,
                                     TrackVenue TEXT,
                                     RaceLaps INTEGER,
                                     constraint k_race_id primary key (race_id),
                                     constraint pract_id foreign key (pract_fid)
                                                 references sessions(ses_id),
                                     constraint quali_id foreign key (quali_fid)
                                                 references sessions(ses_id),
                                     constraint race_id foreign key (race_fid)
                                                 references sessions(ses_id),
                                     constraint seasons_id foreign key (seasons_fid)
                                                 references seasons(season_id)))";
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
                                                references sessions (ses_id)))";
inline constexpr auto CREATE_SESSIONS = R"(create table sessions
                                       (ses_id INTEGER not null,
                                       type TEXT,
                                       constraint k_ses_id primary key (ses_id)))";

DBHelper::DBHelper()
{///todo resolve dublicate conn issue
  dbConn = QSqlDatabase::addDatabase(dbDriverName, dbPath + connName);
  dbConn.setDatabaseName(dbName);
  if (!dbConn.isValid() || !dbConn.open())
    throw std::runtime_error(QString("DataBase init error: ").toStdString()
                             + dbConn.lastError().text().toStdString());
}

DBHelper::~DBHelper()
{
  dbConn.close();
  // QSqlDatabase::removeDatabase(connName);
}

void DBHelper::initDB()
{
  if (dbConn.tables().size() > 0)
    checkSqlError(
      "initDB error",
      QSqlError(QString(), "db is not empty", QSqlError::UnknownError));
  auto resultsError = initResultsTables();
  checkSqlError("initDB error", resultsError);
}
/// todo if error delete all entries with given session id
void DBHelper::addNewResults(const RaceLogInfo &inResults, int sessionId)
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

int DBHelper::addNewRace(const RaceInputData &data)
{
  QSqlQuery q(dbConn);
  q.prepare("insert into " + DBTableNames::Races + R"((pract_fid,
    quali_fid,
    race_fid,
    seasons_fid,
    TrackVenue,
    RaceLaps
    ) values (?, ?, ?, ?, ?, ?))");
  q.addBindValue(data.Pid);
  q.addBindValue(data.Qid);
  q.addBindValue(data.Rid);
  q.addBindValue(data.Seasonid);
  q.addBindValue(data.Track);
  q.addBindValue(data.LapsNum);
  if (!q.exec()) checkSqlError("add new race error", q.lastError());
  return q.lastInsertId().toInt();
}

int DBHelper::addNewSeason(const QString &name)
{
  QSqlQuery q(dbConn);
  QString query{ "insert into " };
  query.append(DBTableNames::Seasons);
  query.append(" (name) values (:name)");
  q.prepare(query);
  q.bindValue(":name", name);
  if (!q.exec()) checkSqlError("add new season error", q.lastError());
  return q.lastInsertId().toInt();
}

void DBHelper::viewTable(QString tableName)
{
  QSqlQuery q(dbConn);
  q.exec("select * from " + tableName);
  while (q.next())
  {
    for (int i = 0; i < q.record().count(); ++i)
    { qDebug() << q.record().fieldName(i) << q.value(i).toString() << " "; }
    qDebug() << '\n';
  }
}

QSqlError DBHelper::initResultsTables()
{
  QSqlQuery init(dbConn);
  if (!init.exec(CREATE_SEASON)) return init.lastError();
  if (!init.exec(CREATE_RACES)) return init.lastError();
  if (!init.exec(CREATE_RESULTS)) return init.lastError();
  if (!init.exec(CREATE_SESSIONS)) return init.lastError();
  return QSqlError();
}

int DBHelper::addNewSession(const QString &type)
{
  QSqlQuery q(dbConn);
  QString query{ "insert into " };
  query.append(DBTableNames::Sessions);
  query.append(" (type) values (?)");
  q.prepare(query);
  q.addBindValue(type);
  if (!q.exec()) checkSqlError("add new session error", q.lastError());
  return q.lastInsertId().toInt();
}

void DBHelper::checkSqlError(const QString &msg, const QSqlError &error)
{
  if (error.type() != QSqlError::NoError)
    throw std::runtime_error(msg.toStdString() + " : "
                             + error.text().toStdString());
}
