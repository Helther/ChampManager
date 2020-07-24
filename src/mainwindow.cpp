#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  resultsW = new resultswindow;
  tabW = new QTabWidget;
  tabW->addTab(resultsW, "Results");
  tabW->addTab(new QWidget(), "Reserved");
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tabW);
  ui->centralwidget->setLayout(layout);
  createActions();
  createMenus();

  userData = new UserData;
  userData->init();
}

MainWindow::~MainWindow()
{
  delete ui;
  delete userData;
}

UserData *MainWindow::getUserData()
{
  assert(userData != nullptr);///debug
  return userData;///todo make inline
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
  seasonD->open();
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

NewRaceDialog::NewRaceDialog(const QVector<SeasonData> &seasons,
                             QWidget *parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Add New Race");
  seasonW = new ChooseSeason(seasons);
  pLabel = new QLabel("Practice log:");
  qLabel = new QLabel("Qualifying log:");
  rLabel = new QLabel("Race log:");
  pFilePath = new QLineEdit();
  qFilePath = new QLineEdit();
  rFilePath = new QLineEdit();
  pBrowseButton = new QPushButton("Browse");
  qBrowseButton = new QPushButton("Browse");
  rBrowseButton = new QPushButton("Browse");
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget(seasonW, 0, 0);
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

  //connect(this, &NewRaceDialog::accepted, this, &NewRaceDialog::test);
  //connect(this, &NewRaceDialog::finished, this, &NewRaceDialog::test);
}

RmSeasonResDialog::RmSeasonResDialog(const QVector<SeasonData> &seasons,
                                     QWidget *parent)
  : QDialog(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Remove Race Results by season");
  seasonW = new ChooseSeason(seasons);
  buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(seasonW);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void RmSeasonResDialog::accept()
{
  ///todo add emmit signal with info argument to connect with slot in main
}

ChooseSeason::ChooseSeason(const QVector<SeasonData> &seasons, QWidget *parent)
  : QWidget(parent)
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
