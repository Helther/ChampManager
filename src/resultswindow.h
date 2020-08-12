#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QWidget>
#include <appdata.h>
#include <QAbstractTableModel>


struct LapsComp
{
  QString driver;
  QString laps;
  QString bestLap;
};

//forward decl
class QTreeView;
class QStandardItemModel;
class QComboBox;
class QLabel;
class QTableView;
class QSqlTableModel;
class DBHelper;
class QMenu;
//
class Resultswindow : public QWidget
{
  Q_OBJECT

public:
  explicit Resultswindow(QWidget *parent = nullptr);
  ~Resultswindow();
  // called when the app is opened to prepare data views
  void init(const QVector<SeasonData> &seasons);
  // sets season combo up to date with main
  void updateSeasons(const QVector<SeasonData> &seasons, bool isInit);

  // sets tree model up to date with db and season combo
public slots:
  // used when results added
  void on_resultsChanged(const SeasonData &season);
  void on_dbReset();

  // used when new season selected
private slots:
  void on_seasonChanged(int seasonComboIndex);
  void on_selectionChanged(const QItemSelection &curSelection);
  void on_treeViewContextMenu(const QPoint &point);
  void on_tableViewContextMenu(const QPoint &point);
  void on_delRaceAct();
  void on_compareLapsAct();
  void on_clearTableSelect();

private:
  //= construction methods
  void layoutSetup();
  void viewWidgetsSetup();
  void setItemHeaderData();
  void createContextMenus();
  //=
  // builds up tree item model with race list
  void updateItemModel(const SeasonData &season);
  // builds up model for race results
  void updateTableModel(int sessionId, bool isRace);
  // sets race view headers names after model rebuild


  SeasonData currentSeason;
  DBHelper *dbHandler;
  // list of db column id to hide in a table view for P,Q session
  const std::array<int, 6> resultsColumnsToHideR{ 0, 1, 3, 4, 10, 13 };
  // for R sessions
  const std::array<int, 10> resultsColumnsToHidePQ{ 0,  1,  3,  4,  9,
                                                    10, 11, 13, 15, 16 };
  //================== Widgets ===================//
  QTreeView *treeView;
  QStandardItemModel *itemModel;
  QTableView *tableView;
  QSqlTableModel *tableModel;
  QLabel *seasonsLabel;
  QComboBox *seasonsCombo;
  //================== Context Menus ===================//
  QMenu *treeMenu;
  QMenu *tableMenu;
};

class LapsCompModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  LapsCompModel(const LapsComp &lapsData, QObject *parent = nullptr);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const override;

private:
  QStringList parseLaps(const QString &lapsString);
  // returns -1 if there was no bestlap
  int getBLapRow() const;
  // convets time string from x.xxx to x:xx.xxx
  QString convertToLapTime(const QString &lapTime) const;

  const QStringList lapTimes;
  const QString bestLap;
  const int rows;
  const int columns;
  const int bLapRow;
};

#endif// RESULTSWINDOW_H
