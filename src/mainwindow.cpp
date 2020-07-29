#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QErrorMessage>
#include <QFileDialog>
#include <parser.h>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  resultsW = new Resultswindow;
  tabW = new QTabWidget;
  tabW->addTab(resultsW, "Results");
  tabW->addTab(new QWidget(), "Reserved");
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tabW);
  ui->centralwidget->setLayout(layout);
  createActions();
  createMenus();

  ///temp while no init
  auto on_destroyed = []() {
    QFile db("CMM.db3");
    if (db.exists()) db.remove();
  };
  connect(this, &QWidget::destroyed, on_destroyed);


  //data init
  initData();
}

MainWindow::~MainWindow()
{
  delete ui;
  delete userData;
}


void MainWindow::on_addNewRace()
{
  NewRaceDialog *addRace = new NewRaceDialog(getUserData()->getSeasons(), this);
  addRace->open();
}

void MainWindow::on_rmSeasonRes()
{
  RmSeasonResDialog *seasonD =
    new RmSeasonResDialog(getUserData()->getSeasons(), this);
  connect(seasonD, &RmSeasonResDialog::accepted, [this]() {
    getUserData()->updateSeasons();
  });
  seasonD->open();
}

void MainWindow::initData()
{
  try
  {
    DBHelper DB;
    const bool isThereDB = DB.initDB();
    userData = new UserData();
    if (!isThereDB) userData->addSeason("Season 1");///first time init
  } catch (std::exception &e)
  {
    DBHelper DBDestroyer;
    DBDestroyer.destroyDB();
    QErrorMessage *msg = new QErrorMessage(this);
    connect(msg, &QErrorMessage::finished, this, &MainWindow::close);
    msg->showMessage(e.what());
    msg->show();
  }
  ///todo add init results
}

void MainWindow::createActions()
{
  newRaceAct = new QAction("Add new race result", this);
  newRaceAct->setStatusTip(tr("Create new race log entry"));
  connect(newRaceAct, &QAction::triggered, this, &MainWindow::on_addNewRace);
  rmSeasonRacesAct = new QAction("delete all season results", this);
  connect(rmSeasonRacesAct,
          &QAction::triggered,
          this,
          &MainWindow::on_rmSeasonRes);
  exitAct = new QAction("Exit");
  connect(exitAct, &QAction::triggered, this, &MainWindow::close);
  licenseAct = new QAction("License info");
  aboutAct = new QAction("About");
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu("File");
  fileMenu->addAction(newRaceAct);
  fileMenu->addAction(rmSeasonRacesAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);
  helpMenu = menuBar()->addMenu("Help");
  helpMenu->addAction(licenseAct);
  helpMenu->addAction(aboutAct);
}
//==========================NewRace=================================//
NewRaceDialog::NewRaceDialog(const QVector<SeasonData> &seasons,
                             QWidget *parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Add New Race");
  seasonW = new ChooseSeason(seasons);
  addSeasonButton = new QPushButton("Add season");
  connect(addSeasonButton,
          &QPushButton::clicked,
          this,
          &NewRaceDialog::on_addSeason);
  pLabel = new QLabel("Select Practice log:");
  qLabel = new QLabel("Select Qualifying log:");
  rLabel = new QLabel("Select Race log:");
  pFilePath = new QLineEdit();
  qFilePath = new QLineEdit();
  rFilePath = new QLineEdit();

  ///todo debug
  pFilePath->setText("/mnt/Media/Dev/PARSER_tests/P.xml");
  qFilePath->setText("/mnt/Media/Dev/PARSER_tests/Q.xml");
  rFilePath->setText("/mnt/Media/Dev/PARSER_tests/R.xml");

  pBrowseButton = new QPushButton("Browse");
  qBrowseButton = new QPushButton("Browse");
  rBrowseButton = new QPushButton("Browse");
  auto getPFilePath = [this]() {
    const auto fileName = QFileDialog::getOpenFileName(this,
                                                       tr("Open File"),
                                                       tr("./"),
                                                       tr("XML Files (*.xml)"));
    this->pFilePath->setText(fileName);
  };
  auto getQFilePath = [this]() {
    const auto fileName = QFileDialog::getOpenFileName(this,
                                                       tr("Open File"),
                                                       tr("./"),
                                                       tr("XML Files (*.xml)"));
    this->qFilePath->setText(fileName);
  };
  auto getRFilePath = [this]() {
    const auto fileName = QFileDialog::getOpenFileName(this,
                                                       tr("Open File"),
                                                       tr("./"),
                                                       tr("XML Files (*.xml)"));
    this->rFilePath->setText(fileName);
  };
  connect(pBrowseButton, &QPushButton::clicked, getPFilePath);
  connect(qBrowseButton, &QPushButton::clicked, getQFilePath);
  connect(rBrowseButton, &QPushButton::clicked, getRFilePath);
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(seasonW, 0, 0);
  mainLayout->addWidget(addSeasonButton, 0, 1);
  mainLayout->addWidget(pLabel, 1, 0);
  mainLayout->addWidget(pFilePath, 1, 1);
  mainLayout->addWidget(pBrowseButton, 1, 2);
  mainLayout->addWidget(qLabel, 2, 0);
  mainLayout->addWidget(qFilePath, 2, 1);
  mainLayout->addWidget(qBrowseButton, 2, 2);
  mainLayout->addWidget(rLabel, 3, 0);
  mainLayout->addWidget(rFilePath, 3, 1);
  mainLayout->addWidget(rBrowseButton, 3, 2);
  mainLayout->addWidget(buttonBox, 4, 0);
  setLayout(mainLayout);
}

void NewRaceDialog::on_addSeason()
{
  AddSeason *newSeason = new AddSeason(this);
  connect(newSeason,
          &AddSeason::added,
          this,
          &NewRaceDialog::updateSeasonsCombo);
  newSeason->open();
}

void NewRaceDialog::updateSeasonsCombo()
{
  seasonW->setSeasons(
    dynamic_cast<MainWindow *>(parent())->getUserData()->getSeasons());
}

void NewRaceDialog::accept()
{
  try
  {
    auto p = QFile(pFilePath->text());
    auto q = QFile(qFilePath->text());
    auto r = QFile(rFilePath->text());
    auto pid = parseFile(PQXmlParser(p));///todo fix p and q equality
    auto qid = parseFile(PQXmlParser(q));
    auto rid = parseFile(RXmlParser(r));
    int seasonId = seasonW->getSeasonData().id;
    DBHelper DB;
    DB.addNewRace(/*RaceInputData*/ { pid, qid, rid, seasonId });

    QDialog::accept();
  } catch (std::exception &e)
  {
    QErrorMessage *msg = new QErrorMessage(this);
    msg->showMessage(QString("can't add the race results:") + e.what());
    msg->show();
    ///todo add revert db changes if failed
  }
}
//==========================RmSeason=================================//
RmSeasonResDialog::RmSeasonResDialog(const QVector<SeasonData> &seasons,
                                     QWidget *parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Remove Race Results by season");
  seasonW = new ChooseSeason(seasons);
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox,
          &QDialogButtonBox::accepted,
          this,
          &RmSeasonResDialog::acceptedSg);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(seasonW);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void RmSeasonResDialog::acceptedSg()
{
  if (seasonW->getSeasons().size() <= 1)
  {
    QErrorMessage *msg = new QErrorMessage(this);///wrap the error call
    msg->showMessage("can't delete the last season");
    msg->show();
    return;
  }
  int currSeason = seasonW->getSeasonData().id;
  DBHelper db;
  db.delSeasonData(currSeason);
  QDialog::accept();
  ///todo add emmit signal with info argument to connect with slot in main to update all gui
}
//==========================ChooseSeason=================================//
ChooseSeason::ChooseSeason(const QVector<SeasonData> &seasons, QWidget *parent)
  : QWidget(parent), seasonsCopy(seasons)
{
  seasonsLabel = new QLabel("Choose a season:");
  seasonsCombo = new QComboBox();
  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(seasonsLabel);
  layout->addWidget(seasonsCombo);
  setLayout(layout);
  for (const auto &i : seasons)
    seasonsCombo->addItem(i.name, QVariant::fromValue(i));
}

SeasonData ChooseSeason::getSeasonData()
{
  return seasonsCombo->currentData().value<SeasonData>();
}

void ChooseSeason::setSeasons(const QVector<SeasonData> &sData)
{
  seasonsCopy = sData;
  seasonsCombo->clear();
  for (auto &&i : seasonsCopy)
    seasonsCombo->addItem(i.name, QVariant::fromValue(i));
}
//==========================UserData=================================//
UserData::UserData() { init(); }

void UserData::init() { updateSeasons(); }

void UserData::addSeason(const QString &name)
{
  DBHelper db;
  int id = db.addNewSeason(name);
  seasons.push_back(/*SeasonData*/ { id, name });
}

void UserData::updateSeasons()
{
  DBHelper db;
  if (!seasons.isEmpty()) seasons.clear();
  auto seasonsTable = db.getData(DBTableNames::Seasons);
  for (const auto &i : seasonsTable)
  {
    SeasonData newSData;
    newSData.id = i.at(0).toInt();
    newSData.name = i.at(1).toString();
    seasons.push_back(newSData);
  }
}
//==========================AddSeason=================================//
AddSeason::AddSeason(QWidget *parent) : QDialog(parent)
{
  setWindowTitle("Add New Season");
  lineEdit = new QLineEdit;
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(lineEdit);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void AddSeason::accept()
{
  const auto name = lineEdit->text();
  auto p = dynamic_cast<NewRaceDialog *>(parent());
  const auto seasons =
    dynamic_cast<MainWindow *>(p->parent())->getUserData()->getSeasons();
  if (std::find_if(seasons.begin(),
                   seasons.end(),
                   [name](SeasonData s) { return s.name == name; })
        == seasons.end()
      && name.size() > 0)
  {
    dynamic_cast<MainWindow *>(p->parent())->getUserData()->addSeason(name);
    QDialog::accept();
    emit added();
    return;
  }
  QErrorMessage *msg = new QErrorMessage(this);
  msg->showMessage("can't add the season");
  msg->show();
}
