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
#include <QHBoxLayout>

Resultswindow::Resultswindow(QWidget *parent)
  : QWidget(parent), treeView(new QTreeView), itemModel(new QStandardItemModel),
    tableView(new QTableView), tableModel(new QSqlTableModel),
    seasonsLabel(new QLabel("Results for season:")), seasonsCombo(new QComboBox)
{
  layoutSetup();
  setItemHeaderData();

  treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

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
}

void Resultswindow::on_seasonChanged(int seasonComboIndex)
{
  auto seasonData =
    seasonsCombo->itemData(seasonComboIndex).value<SeasonData>();
  if (seasonData.id != currentSeason.id)// do nothing if still the same season
  {
    updateItemModel(seasonData);
    currentSeason = seasonData;
  }
}

void Resultswindow::on_selectionChanged(const QItemSelection &curSelection)
{
  qDebug() << "sel changed" << curSelection.indexes();
}

void Resultswindow::updateItemModel(const SeasonData &season)
{
  Perf p("updateItemModel func");
  if (itemModel->rowCount() != 0) itemModel->clear();
  DBHelper db;
  auto data = db.getRaceData(season.id);
  for (int i = 0; i < data.size(); ++i)
  {
    QStandardItem *root = itemModel->invisibleRootItem();
    QStandardItem *race = new QStandardItem{ data.at(i).track };
    race->setData(QVariant::fromValue(data.at(i).raceId));
    root->insertRow(i,
                    { race,
                      new QStandardItem{ QString::number(data.at(i).laps) },
                      new QStandardItem{ data.at(i).date } });
    for (const auto &session : data.at(i).sessions)
    {

      QStandardItem *sess = new QStandardItem{ session.second };// set type
      sess->setData(QVariant::fromValue(session.first));// set id
      race->appendRow(sess);
    }
  }
  treeView->setModel(itemModel);

  setItemHeaderData();
  treeView->resizeColumnToContents(0);
}

void Resultswindow::updateTableModel(int sessionId)
{
  qDebug() << "updateTableModel with session id " << sessionId;
}

void Resultswindow::setItemHeaderData()
{
  itemModel->setHorizontalHeaderItem(0, new QStandardItem("Track/Session"));
  itemModel->setHorizontalHeaderItem(1, new QStandardItem("Num Of Laps"));
  itemModel->setHorizontalHeaderItem(2, new QStandardItem("Date-Time"));
}


void Resultswindow::layoutSetup()
{
  QGridLayout *layout = new QGridLayout;
  QHBoxLayout *seasonLayout = new QHBoxLayout;
  seasonLayout->addWidget(seasonsLabel);
  seasonLayout->addWidget(seasonsCombo);
  layout->addLayout(seasonLayout, 0, 0);
  layout->addWidget(treeView, 1, 0);
  layout->addWidget(tableView, 0, 1, 2, Qt::AlignLeft);
  setLayout(layout);
}
