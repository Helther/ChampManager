#pragma once

#include <QWidget>
#include <appdata.h>
#include <QAbstractTableModel>
#include <lapscomparewidget.h>
#include <array>


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
  void on_currentChanged(const QModelIndex &selected);
  void on_treeViewContextMenu(const QPoint &point);
  void on_tableViewContextMenu(const QPoint &point);
  void on_delRaceAct();
  void on_compareLapsAct();
  void on_clearTableSelect();
  void on_delAllRacesAct();

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
  [[nodiscard]] QVector<LapsComp> getLapsData();

  SeasonData currentSeason;
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

