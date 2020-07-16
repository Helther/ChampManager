#include "dbhelper.h"

inline constexpr auto CREATE_SEASON = R"(create table seasons
                                      (season_id INTEGER not null,
                                      name TEXT,
                                      constraint k_season_id primary key (season_id)))";
inline constexpr auto CREATE_RACES = R"(create table races
                                     (race_id INTEGER not null,
                                     pract_res_fid INTEGER not null,
                                     quali_res_fid INTEGER not null,
                                     race_res_fid INTEGER not null,
                                     seasons_fid INTEGER not null,
                                     TrackVenue TEXT,
                                     RaceLaps INTEGER,
                                     constraint k_race_id primary key (race_id),
                                     constraint pract_res_id foreign key (pract_res_fid)
                                                 references race_results(res_id),
                                     constraint quali_res_id foreign key (quali_res_fid)
                                                 references race_results(res_id),
                                     constraint race_res_id foreign key (race_res_fid)
                                                 references race_results(res_id),
                                     constraint seasons_id foreign key (seasons_fid)
                                                 references seasons(season_id)))";
inline constexpr auto CREATE_RESULTS = R"(create table race_results
                                       (res_id INTEGER not null,
                                       type TEXT,
                                       Name TEXT,
                                       VehFile TEXT,
                                       VehName TEXT,
                                       CarType TEXT,
                                       CarClass TEXT,
                                       CarNumber TEXT,
                                       TeamName TEXT,
                                       GridPos INTEGER,
                                       Position INTEGER,
                                       ClassGridPos INTEGER,
                                       ClassPosition INTEGER,
                                       Lap TEXT,
                                       BestLapTime REAL,
                                       Pitstops INTEGER,
                                       FinishStatus TEXT,
                                       constraint k_res_id primary key (res_id)))";

DBHelper::DBHelper()
{
  dbConn = QSqlDatabase::addDatabase(dbDriverName, dbPath + connName);
  if (!initConnection())
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
  ///todo temp season init
  addNewSeason("default season");
}

void DBHelper::addNewSeason(const QString &name)
{
  QSqlQuery q(dbConn);
  q.prepare("insert into seasons (name) values (:name)");
  q.bindValue(":name", name);
  if (!q.exec()) checkSqlError("add new season error", q.lastError());
}

bool DBHelper::initConnection()
{
  dbConn.setDatabaseName(dbName);
  return dbConn.isValid() && dbConn.open();
}

QSqlError DBHelper::initResultsTables()
{
  QSqlQuery init(dbConn);
  if (!init.exec(CREATE_SEASON)) return init.lastError();
  if (!init.exec(CREATE_RACES)) return init.lastError();
  if (!init.exec(CREATE_RESULTS)) return init.lastError();
  //  init.exec("insert into seasons (name) values (\"k\")");
  //  init.exec("insert into seasons (name) values (\"kkk\")");
  //  init.exec("select * from seasons");
  //  while (init.next())
  //  {
  //    for (int i = 0; i < init.record().count(); ++i)
  //    {
  //      qDebug() << init.record().fieldName(i) << init.value(i).toString() << "";
  //    }
  //    qDebug() << '\n';
  //  }
  return QSqlError();
}

void DBHelper::checkSqlError(const QString &msg, const QSqlError &error)
{
  if (error.type() != QSqlError::NoError)
    throw std::runtime_error(msg.toStdString() + " : "
                             + error.text().toStdString());
}
