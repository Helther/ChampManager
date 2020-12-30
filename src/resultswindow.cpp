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
#include <QMessageBox>

#define MAX_LAPSCOMPARE_ROW_SIZE 5
#define POSITION_COL_NUM 10

Resultswindow::Resultswindow(QWidget *parent)
  : QWidget(parent), treeView(new QTreeView),
    itemModel(new QStandardItemModel(this)), tableView(new QTableView),
    tableModel(new QSqlTableModel(this, dbObj.getDBConnection())),
    seasonsLabel(new QLabel("Results for season:")),
    seasonsCombo(new QComboBox), treeMenu(new QMenu), tableMenu(new QMenu)
{
  layoutSetup();
  viewWidgetsSetup();
  setItemHeaderData();
  createContextMenus();
  connect(seasonsCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          QOverload<int>::of(&Resultswindow::on_seasonChanged));
}

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

void Resultswindow::on_dbReset()
{
  tableModel->clear();
  itemModel->clear();
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
  }
}

void Resultswindow::on_treeViewContextMenu(const QPoint &point)
{
  auto index = treeView->indexAt(point);
  if (index.isValid())
  {
    auto currentItem = itemModel->itemFromIndex(index);
    if (currentItem != nullptr && currentItem->parent() == nullptr)
      treeMenu->exec(treeView->viewport()->mapToGlobal(point));
  }
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
      try
      {
        int raceId = currentItem->data().value<int>();
        dbObj.delEntryFromTable(DBTableNames::Races, "race_id", raceId);
        on_resultsChanged(currentSeason);
      } catch (std::exception &e)
      {
        QMessageBox::critical(this, "Deletion Error: ", e.what());
      }
    }
  }
}

void Resultswindow::on_compareLapsAct()
{
  Perf p("compare laps");///todo temp
  LapsCompareWidget *lapsW = nullptr;
  try
  {
    lapsW = new LapsCompareWidget(getLapsData());
    lapsW->show();
  } catch (std::exception &e)
  {
    QMessageBox::critical(this,
                          "Error",
                          QString("Can't show comparison: ") + e.what());
  }
}

void Resultswindow::on_clearTableSelect() { tableView->clearSelection(); }

void Resultswindow::on_delAllRacesAct()
{
  auto reply = QMessageBox::question(
    this,
    "Confirm deletion",
    "Are you  sure you want to delete all races data, in the current season?",
    QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes)
  {

    try
    {
      dbObj.delSeasonRaces(currentSeason.id);
      on_resultsChanged(currentSeason);
    } catch (std::exception &e)
    {
      QMessageBox::critical(this, "Deletion Error: ", e.what());
    }
  }
}

void Resultswindow::updateItemModel(const SeasonData &season)
{
  Perf p("updateItemModel func");///todo temp
  if (itemModel->rowCount() != 0) itemModel->clear();
  try
  {
    auto raceData = dbObj.getRaceData(season.id);
    for (int i = 0; i < raceData.size(); ++i)
    {
      QStandardItem *root = itemModel->invisibleRootItem();
      QStandardItem *race = new QStandardItem{ raceData.at(i).track };
      race->setData(QVariant::fromValue(raceData.at(i).raceId));
      root->insertRow(
        i,
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
  } catch (std::exception &e)
  {
    QMessageBox::critical(this, "Deletion Error: ", e.what());
  }
}

void Resultswindow::updateTableModel(int sessionId, bool isRace)
{
  Perf p("updateTableModel func");/// todo temp
  tableModel->setTable(DBTableNames::RaceRes);
  tableModel->setFilter(QString("session_fid = \"%1\"").arg(sessionId));
  tableModel->setSort(POSITION_COL_NUM, Qt::SortOrder::AscendingOrder);
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

QVector<LapsComp> Resultswindow::getLapsData()
{
  const int nameColId = 2;
  const int lapsColId = 13;
  const int bestLapId = 14;
  QVector<LapsComp> lapsData;
  auto indexes = tableView->selectionModel()->selectedRows();
  if (indexes.isEmpty()) return lapsData;
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
    throw std::runtime_error("Compare Laps Creation error");
  }
  return lapsData;
}

void Resultswindow::setItemHeaderData()
{
  itemModel->setHorizontalHeaderItem(0, new QStandardItem("Track/Session"));
  itemModel->setHorizontalHeaderItem(1, new QStandardItem("Num Of Laps"));
  itemModel->setHorizontalHeaderItem(2, new QStandardItem("Date-Time"));
  for (int i = 0; i < itemModel->columnCount(); ++i)
    treeView->resizeColumnToContents(i);
}

void Resultswindow::createContextMenus()
{
  treeMenu->addAction("Delete race result",
                      this,
                      &Resultswindow::on_delRaceAct);
  treeMenu->addSeparator();
  treeMenu->addAction("Delete all races in season",
                      this,
                      &Resultswindow::on_delAllRacesAct);
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

void Resultswindow::viewWidgetsSetup()
{
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
}
