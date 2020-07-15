#include "dbhelper.h"

inline constexpr auto CREATE_SEASON = R"(create table seasons
                                      (season_id int,
                                      name varchar(40),
                                      constraint k_season_id primary key (season_id)))";
inline constexpr auto CREATE_RACES = R"(create table races
                                     (race_id int not null,
                                     pract_res_fid int not null,
                                     quali_res_fid int not null,
                                     race_res_fid int not null,
                                     seasons_fid int not null,
                                     TrackVenue varchar(40),
                                     RaceLaps int,
                                     constraint k_race_id primary key (race_id),
                                     constraint pract_res_id foreign key (pract_res_fid)
                                                 references race_results(res_id)
                                     constraint quali_res_id foreign key (quali_res_fid)
                                                 references race_results(res_id)
                                     constraint race_res_id foreign key (race_res_fid)
                                                 references race_results(res_id)
                                     constraint seasons_id foreign key (seasons_fid)
                                                 references seasons(season_id)))";
inline constexpr auto CREATE_RESULTS = R"(create table race_results
                                       (res_id int not null,
                                       type varchar(20),
                                       constraint k_res_id primary key (res_id)
                                       ))";

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

QSqlError DBHelper::initDB()
{
  if (dbConn.tables().size() > 0)
    return QSqlError(QString(),
                     "initDB error: db is not empty",
                     QSqlError::UnknownError);
  auto resultsError = initResultsTables();
  if (resultsError.type() != QSqlError::NoError) return resultsError;
  return QSqlError();
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
  init.exec("insert into seasons (name) values (k)");
  init.exec("select * from seasons");
  while (init.next())
  {
    for (int i = 0; i < init.record().count(); ++i)
    {
      qDebug() << init.record().fieldName(i) << init.value(i).toString() << "";
    }
    qDebug() << '\n';
  }
  return QSqlError();
}
