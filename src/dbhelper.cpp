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

bool DBHelper::initDB()
{
  QFile db(dbName);
  if (!db.exists() || dbConn.tables().size() == 0)
  {
    auto resultsError = initResultsTables();
    checkSqlError("initDB error", resultsError);
    return false;///todo add table existence check
  }
  return true;
}

void DBHelper::destroyDB()
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
  ///q.addBindValue(); todo
  ///q.addBindValue(); todo
  if (!q.exec()) checkSqlError("add new race error", q.lastError());
  return q.lastInsertId().toInt();
}

int DBHelper::addNewSeason(const QString &name)
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

void DBHelper::viewTable(const QString &tableName)
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

QVector<QVector<QVariant>> DBHelper::getData(const QString &tableName)
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

void DBHelper::delEntryFromTable(const QString &table,
                                 const QString &idCol,
                                 int id)
{
  QSqlQuery q(dbConn);
  QString query{ "delete from " };
  query.append(table);
  query.append(" where " + idCol + " = " + QString::number(id));
  if (!q.exec(query)) checkSqlError("delete table entry error", q.lastError());
}

void DBHelper::checkSqlError(const QString &msg, const QSqlError &error)
{
  if (error.type() != QSqlError::NoError)
    throw std::runtime_error(msg.toStdString() + " : "
                             + error.text().toStdString());
}
