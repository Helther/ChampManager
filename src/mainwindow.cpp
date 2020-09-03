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
#include <QMessageBox>

#define DCTEST_DATA_DIR TESTDATA_PATH

#define APP_ICON "://better_icon.png"

inline constexpr auto aboutAppText =
  R"(Championship Manager is a desktop application
for managing racing carrier by proccessing logs
and settings files provided by rFactor 2 app.

It uses Qt Framework v5 and falls under it's license GNU (L)GPL.

Copyright (C) 2020 Gusev Anton, email address: kaeldevop@gmai.com.)";


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow), tabW(new QTabWidget),
    resultsW(new Resultswindow)
{
  //data init
  initData();
  ui->setupUi(this);
  tabW->addTab(resultsW, "Results");
  tabW->addTab(new QWidget(), "Reserved");///todo temp
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tabW);
  ui->centralwidget->setLayout(layout);
  createActions();
  createMenus();
  setWindowIcon(QIcon(APP_ICON));

  connect(this,
          &MainWindow::on_resultsChanged,
          resultsW,
          &Resultswindow::on_resultsChanged);
  connect(this,
          &MainWindow::on_dbReseted,
          resultsW,
          &Resultswindow::on_dbReset);
}

MainWindow::~MainWindow()
{
  delete ui;
  delete userData;
}

void MainWindow::on_seasonsChanged(const SeasonData)
{
  resultsW->updateSeasons(getUserData()->getSeasons(), false);
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

void MainWindow::about()
{
  QDialog *aboutDialog = new QDialog;
  aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
  QDialogButtonBox *okButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(okButton, &QDialogButtonBox::accepted, aboutDialog, &QDialog::accept);
  QLabel *textLabel = new QLabel;
  QLabel *picLabel = new QLabel;
  textLabel->setText(aboutAppText);
  auto pic = QPixmap(APP_ICON);
  picLabel->setPixmap(pic.scaledToWidth(128));
  QGridLayout *layout = new QGridLayout;
  layout->addWidget(textLabel, 0, 0);
  layout->addWidget(picLabel, 0, 1);
  layout->addWidget(okButton, 1, 0, 1, 2, Qt::AlignCenter);
  aboutDialog->setLayout(layout);
  aboutDialog->show();
}

void MainWindow::license() { QMessageBox::aboutQt(this, tr("About Qt")); }

void MainWindow::resetBdData()
{
  auto reply = QMessageBox::question(
    this,
    "Confirm deletion",
    "Are you  sure you want to delete all data, including profile settings?",
    QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes)
  {
    try
    {
      userData->init(true);// reset user data
      resultsW->init(getUserData()->getSeasons());
      emit on_dbReseted();
    } catch (std::exception &e)
    {
      QMessageBox::critical(this, "Reset Data Error", e.what());
    }
  }
}

void MainWindow::initData()
{
  try
  {
    userData = new UserData();
    resultsW->init(getUserData()->getSeasons());
  } catch (std::exception &e)
  {
    QErrorMessage *msg = new QErrorMessage(this);
    connect(msg, &QErrorMessage::finished, this, &MainWindow::close);
    msg->showMessage(e.what());
    msg->show();
  }
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
  connect(licenseAct, &QAction::triggered, this, &MainWindow::license);
  aboutAct = new QAction("About");
  connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
  resetAllDataAct = new QAction("Reset all data");
  connect(resetAllDataAct, &QAction::triggered, this, &MainWindow::resetBdData);
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu("File");
  fileMenu->addAction(newRaceAct);
  fileMenu->addAction(rmSeasonRacesAct);
  fileMenu->addSeparator();
  fileMenu->addAction(resetAllDataAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);
  helpMenu = menuBar()->addMenu("Help");
  helpMenu->addAction(licenseAct);
  helpMenu->addAction(aboutAct);
}
//==========================NewRace=================================//
NewRaceDialog::NewRaceDialog(const QVector<SeasonData> &seasons,
                             QWidget *parent)
  : QDialog(parent), seasonW(new ChooseSeason(seasons)),
    addSeasonButton(new QPushButton("Add season")),
    pLabel(new QLabel("Select Practice log:")),
    qLabel(new QLabel("Select Qualifying log:")),
    rLabel(new QLabel("Select Race log:")), pFilePath(new QLineEdit),
    qFilePath(new QLineEdit), rFilePath(new QLineEdit),
    pBrowseButton(new BrowseButton(pFilePath, "XML Files (*.xml")),
    qBrowseButton(new BrowseButton(qFilePath, "XML Files (*.xml)")),
    rBrowseButton(new BrowseButton(rFilePath, "XML Files (*.xml)"))
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Add New Race");
  connect(addSeasonButton,
          &QPushButton::clicked,
          this,
          &NewRaceDialog::on_addSeason);
  ///todo debug
  pFilePath->setText(QString(DCTEST_DATA_DIR) + QString("P.xml"));
  qFilePath->setText(QString(DCTEST_DATA_DIR) + QString("Q.xml"));
  rFilePath->setText(QString(DCTEST_DATA_DIR) + QString("R.xml"));
  /// end debug
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(this,
          &NewRaceDialog::addedRace,
          static_cast<MainWindow *>(parent),
          &MainWindow::on_resultsChanged);

  layoutSetup();
}

void NewRaceDialog::on_addSeason()
{
  AddSeason *newSeason = new AddSeason(this);
  connect(newSeason,
          &AddSeason::addedSeason,
          this,
          &NewRaceDialog::updateSeasonsCombo);
  newSeason->open();
}

void NewRaceDialog::updateSeasonsCombo(const SeasonData &season)
{
  auto currentSeasons = seasonW->getSeasons();
  currentSeasons.push_back(season);
  seasonW->setSeasons(currentSeasons);
}

void NewRaceDialog::accept()
{
  DBHelper db;///todo may throw
  try
  {
    Perf perf("add new race func");///todo temp
    int seasonId = seasonW->getSeasonData().id;
    // transact lifetime is try block
    db.transactionStart();//------------ transact start
    int raceId = db.addNewRace(seasonId);
    int pSessionId = addSession<PXmlParser>(pFilePath->text(), raceId, db);
    int qSessionId = addSession<QXmlParser>(qFilePath->text(), raceId, db);
    int rSessionId = addSession<RXmlParser>(rFilePath->text(), raceId, db);
    db.checkSessionsValidity({ pSessionId, qSessionId, rSessionId });
    db.transactionCommit();//------------ transact commit
    QDialog::accept();
    emit addedRace(seasonW->getSeasonData());
  } catch (std::exception &e)
  {
    db.transactionRollback();
    QMessageBox::critical(this, "Add Results Error", e.what());
  }
}

void NewRaceDialog::layoutSetup()
{
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
  connect(this,
          &RmSeasonResDialog::removed,
          static_cast<MainWindow *>(parent),
          &MainWindow::on_seasonsChanged);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(seasonW);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void RmSeasonResDialog::acceptedSg()
{
  try
  {
    if (seasonW->getSeasons().size() <= 1)
      throw std::runtime_error("can't delete the only season");
    auto currSeason = seasonW->getSeasonData();
    DBHelper db;
    db.delSeasonData(currSeason.id);
    QDialog::accept();
    emit removed(currSeason);
  } catch (std::exception &e)
  {
    QMessageBox::critical(this, "Error", e.what());
  }
}
//==========================ChooseSeason=================================//
ChooseSeason::ChooseSeason(const QVector<SeasonData> &seasons, QWidget *parent)
  : QWidget(parent), seasonsCopy(seasons),
    seasonsLabel(new QLabel("Choose a season:")), seasonsCombo(new QComboBox)
{
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

//==========================AddSeason=================================//
AddSeason::AddSeason(QWidget *parent) : QDialog(parent)
{
  setWindowTitle("Add New Season");
  lineEdit = new QLineEdit;
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  auto p = dynamic_cast<NewRaceDialog *>(parent);
  connect(this,
          &AddSeason::addedSeason,
          static_cast<MainWindow *>(p->parent()),
          &MainWindow::on_seasonsChanged);

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
    static_cast<MainWindow *>(p->parent())->getUserData()->getSeasons();
  if (std::find_if(seasons.begin(),
                   seasons.end(),
                   [name](SeasonData s) { return s.name == name; })
        == seasons.end()
      && name.size() > 0)
  {
    auto newSeason =
      static_cast<MainWindow *>(p->parent())->getUserData()->addSeason(name);
    QDialog::accept();
    emit addedSeason(newSeason);
    return;
  }
  QMessageBox::critical(this, "Error", "can't add the season: invalid name");
}


BrowseButton::BrowseButton(QLineEdit *pathLine,
                           const QString &fileFilter,
                           QWidget *parent)
  : QPushButton("Browse", parent), pathLinePtr(pathLine), filter(fileFilter)
{
  if (pathLinePtr == nullptr)
    throw std::runtime_error("Browse bad path line obj");
  connect(this, &QPushButton::clicked, this, &BrowseButton::chooseFile);
}

void BrowseButton::chooseFile()
{
  const auto fileName =
    QFileDialog::getOpenFileName(this, "Select a File", "./", filter);
  if (!fileName.isNull())
  {
    if (pathLinePtr == nullptr)
      throw std::runtime_error("Browse bad path line obj");
    pathLinePtr->setText(fileName);
  }
}
