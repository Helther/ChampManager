#include "resultswindow.h"
#include <dbhelper.h>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QComboBox>
#include <QLabel>
#include <QTableView>
#include <QSqlTableModel>
#include <QGridLayout>
#include <QGroupBox>
#include <QMenu>

Resultswindow::Resultswindow(QWidget *parent)
  : QWidget(parent), dbHandler(new DBHelper), treeView(new QTreeView),
    itemModel(new QStandardItemModel(this)), tableView(new QTableView),
    tableModel(new QSqlTableModel(this, dbHandler->getDBConnection())),
    seasonsLabel(new QLabel("Results for season:")),
    seasonsCombo(new QComboBox), treeMenu(new QMenu), tableMenu(new QMenu)
{
  layoutSetup();
  setItemHeaderData();
  // todo wrap around in view setter
  treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  treeView->setAlternatingRowColors(true);
  treeView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(treeView,
          &QTreeView::customContextMenuRequested,
          this,
          &Resultswindow::on_treeViewContextMenu);

  tableView->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableView->setAlternatingRowColors(true);
  tableView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(tableView,
          &QTableView::customContextMenuRequested,
          this,
          &Resultswindow::on_tableViewContextMenu);

  connect(seasonsCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          QOverload<int>::of(&Resultswindow::on_seasonChanged));
  createContextMenus();
}

Resultswindow::~Resultswindow() { delete dbHandler; }

void Resultswindow::init(const QVector<SeasonData> &seasons)
{
  updateSeasons(seasons, true);
  auto seasonData =
    seasonsCombo->itemData(seasonsCombo->currentIndex()).value<SeasonData>();
  updateItemModel(seasonData);
  connect(treeView->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this,
          &Resultswindow::on_selectionChanged);

  currentSeason = seasonData;
}

void Resultswindow::updateSeasons(const QVector<SeasonData> &seasons,
                                  bool isInit)
{
  seasonsCombo->clear();
  for (auto &&i : seasons)
    seasonsCombo->addItem(i.name, QVariant::fromValue(i));
  if (!isInit) on_seasonChanged(seasonsCombo->currentIndex());
}

void Resultswindow::on_resultsChanged(const SeasonData &season)
{
  if (currentSeason.id == season.id)// do nothing if added to a different season
    updateItemModel(season);
  tableModel->clear();
}

void Resultswindow::on_seasonChanged(int seasonComboIndex)
{
  auto seasonData =
    seasonsCombo->itemData(seasonComboIndex).value<SeasonData>();
  if (seasonData.id != currentSeason.id)// do nothing if still the same season
  {
    updateItemModel(seasonData);
    currentSeason = seasonData;
    tableModel->clear();
  }
}

void Resultswindow::on_selectionChanged(const QItemSelection &curSelection)
{
  auto currentItem = itemModel->itemFromIndex(curSelection.indexes().first());
  if (currentItem != nullptr && currentItem->parent() != nullptr)
  {
    auto sessionData = currentItem->data().value<QPair<int, QString>>();
    bool isRace = sessionData.second
                  == parserConsts::FileTypes::getFileTypeById(
                    parserConsts::FileTypes::FileType::RaceLog);
    updateTableModel(sessionData.first, isRace);
    return;
  }
  qDebug() << "bad selection changed";
}

void Resultswindow::on_treeViewContextMenu(const QPoint &point)
{
  auto index = treeView->indexAt(point);
  if (index.isValid())
  {
    auto currentItem = itemModel->itemFromIndex(index);
    if (currentItem != nullptr && currentItem->parent() == nullptr)
      treeMenu->exec(treeView->viewport()->mapToGlobal(point));
    return;
  }
  qDebug() << "bad selection";
}

void Resultswindow::on_tableViewContextMenu(const QPoint &point)
{
  auto index = tableView->indexAt(point);
  if (index.isValid())
    tableMenu->exec(tableView->viewport()->mapToGlobal(point));
}

void Resultswindow::on_delRaceAct()
{
  auto index = treeView->selectionModel()->selection().indexes().first();
  if (index.isValid())
  {
    auto currentItem = itemModel->itemFromIndex(index);
    if (currentItem != nullptr)
    {
      int raceId = currentItem->data().value<int>();
      DBHelper db;
      db.delEntryFromTable(DBTableNames::Races, "race_id", raceId);
      on_resultsChanged(currentSeason);
      return;
    }
  }
  qDebug() << "bad del race act";
}

void Resultswindow::on_compareLapsAct()
{
  Perf p("compare laps");
  const int nameColId = 2;
  const int lapsColId = 13;
  const int bestLapId = 14;
  auto indexes = tableView->selectionModel()->selectedRows();
  if (indexes.isEmpty()) return;
  QVector<LapsComp> lapsData;
  for (const auto &i : indexes)
  {
    if (i.isValid())
    {
      auto name = tableModel->index(i.row(), nameColId);
      auto laps = tableModel->index(i.row(), lapsColId);
      auto bLap = tableModel->index(i.row(), bestLapId);
      if (name.isValid() && laps.isValid() && bLap.isValid())
        lapsData.push_back({ name.data().toString(),
                             laps.data().toString(),
                             bLap.data().toString() });
      continue;
    }
    qDebug() << "bad compare data gather";
    return;///todo
  }

  QWidget *lapsW = new QWidget;
  lapsW->setAttribute(Qt::WA_DeleteOnClose);
  lapsW->setWindowTitle("Lap Times Compare");
  QHBoxLayout *lapsLayout = new QHBoxLayout;
  for (const auto &i : lapsData)
  {
    LapsCompModel *lapsModel = new LapsCompModel(i);
    QTableView *lapsView = new QTableView;
    lapsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lapsView->setAlternatingRowColors(true);
    lapsView->setModel(lapsModel);
    lapsView->resizeColumnsToContents();
    QGroupBox *lapsGroup = new QGroupBox;
    QGridLayout *lapsGrid = new QGridLayout;
    lapsGrid->addWidget(new QLabel("Driver: " + i.driver), 0, 0);
    lapsGrid->addWidget(lapsView, 1, 0, 1, 2);
    lapsGroup->setLayout(lapsGrid);
    lapsLayout->addWidget(lapsGroup);
  }
  lapsW->setLayout(lapsLayout);
  lapsW->show();/// todo add sizing and stretch
}

void Resultswindow::on_clearTableSelect() { tableView->clearSelection(); }

void Resultswindow::updateItemModel(const SeasonData &season)
{
  Perf p("updateItemModel func");
  if (itemModel->rowCount() != 0) itemModel->clear();
  DBHelper db;
  auto raceData = db.getRaceData(season.id);
  for (int i = 0; i < raceData.size(); ++i)
  {
    QStandardItem *root = itemModel->invisibleRootItem();
    QStandardItem *race = new QStandardItem{ raceData.at(i).track };
    race->setData(QVariant::fromValue(raceData.at(i).raceId));
    root->insertRow(i,
                    { race,
                      new QStandardItem{ QString::number(raceData.at(i).laps) },
                      new QStandardItem{ raceData.at(i).date } });
    for (const auto &session : raceData.at(i).sessions)
    {

      QStandardItem *sess = new QStandardItem{ session.second };// set type
      sess->setData(QVariant::fromValue(
        QPair<int, QString>(session.first, session.second)));// set id
      race->appendRow(sess);
    }
  }
  treeView->setModel(itemModel);

  setItemHeaderData();
  for (int i = 0; i < itemModel->columnCount(); ++i)
    treeView->resizeColumnToContents(i);
}

void Resultswindow::updateTableModel(int sessionId, bool isRace)
{
  Perf p("updateTableModel func");
  tableModel->setTable(DBTableNames::RaceRes);
  tableModel->setFilter(QString("session_fid = \"%1\"").arg(sessionId));
  tableModel->setSort(10, Qt::SortOrder::AscendingOrder);
  tableModel->select();
  tableView->setModel(tableModel);
  if (isRace)
    for (const auto &i : resultsColumnsToHideR)
      tableView->setColumnHidden(i, true);
  else
    for (const auto &i : resultsColumnsToHidePQ)
      tableView->setColumnHidden(i, true);
  tableView->resizeColumnsToContents();
}

void Resultswindow::setItemHeaderData()
{
  itemModel->setHorizontalHeaderItem(0, new QStandardItem("Track/Session"));
  itemModel->setHorizontalHeaderItem(1, new QStandardItem("Num Of Laps"));
  itemModel->setHorizontalHeaderItem(2, new QStandardItem("Date-Time"));
}

void Resultswindow::createContextMenus()
{
  treeMenu->addAction("Delete race result",
                      this,
                      &Resultswindow::on_delRaceAct);

  tableMenu->addAction("Compare selected lapsTimes",
                       this,
                       &Resultswindow::on_compareLapsAct);
  tableMenu->addAction("Clear selection",
                       this,
                       &Resultswindow::on_clearTableSelect);
}


void Resultswindow::layoutSetup()
{
  QGridLayout *mainLayout = new QGridLayout;
  QGroupBox *races = new QGroupBox;
  QGroupBox *results = new QGroupBox;

  QGridLayout *racesLayout = new QGridLayout;
  racesLayout->addWidget(seasonsLabel, 0, 0);
  racesLayout->addWidget(seasonsCombo, 0, 1);
  racesLayout->addWidget(treeView, 1, 0, 1, 2);
  races->setLayout(racesLayout);

  QGridLayout *resultsLayout = new QGridLayout;
  resultsLayout->addWidget(new QLabel("Race Results Details"), 0, 0);
  resultsLayout->addWidget(tableView, 1, 0, 1, 2);
  results->setLayout(resultsLayout);

  mainLayout->addWidget(races, 0, 0);
  mainLayout->addWidget(results, 0, 1, 1, 2);
  setLayout(mainLayout);
}

LapsCompModel::LapsCompModel(const LapsComp &lapsData, QObject *parent)
  : QAbstractTableModel(parent), lapTimes(parseLaps(lapsData.laps)),
    bestLap(lapsData.bestLap), rows(lapTimes.size()), columns(2),
    bLapRow(getBLapRow())
{}///todo check for leaks

int LapsCompModel::rowCount(const QModelIndex & /*parent*/) const
{
  return rows;
}

int LapsCompModel::columnCount(const QModelIndex & /*parent*/) const
{
  return columns;
}

QVariant LapsCompModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();
  // generate a log message when this method gets called
  for (int i = 0; i < lapTimes.size(); ++i)
  {
    switch (role)
    {
    case Qt::DisplayRole:
      if (row == i && col == 0) return QString(QString::number(i + 1));
      if (row == i && col == 1) return lapTimes.at(i);
      break;
    case Qt::FontRole:
      if (row == bLapRow && col == 1)
      {//change font only for bestlap
        QFont boldFont;
        boldFont.setBold(true);
        return boldFont;
      }
      break;
    default:
      break;
    }
  }
  return QVariant();
}

QVariant LapsCompModel::headerData(int section,
                                   Qt::Orientation orientation,
                                   int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return QString("№");
    case 1:
      return QString("time in sec");
    }
  }
  return QVariant();
}

QStringList LapsCompModel::parseLaps(const QString &lapsString)
{
  return lapsString.split(", ");
}

int LapsCompModel::getBLapRow()
{
  for (int i = 0; i < lapTimes.size(); ++i)
    if (abs(bestLap.toDouble() - lapTimes.at(i).toDouble()) < 0.002) return i;
  throw std::runtime_error("lapsCompModel: cant match best lap");
}
