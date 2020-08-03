#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QWidget>
#include <appdata.h>

//forward decl
class QTreeView;
class QStandardItemModel;
class QComboBox;
class QLabel;
class QTableView;
class QSqlTableModel;
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

  // used when new season selected
private slots:
  void on_seasonChanged(int seasonComboIndex);
  void on_selectionChanged(const QItemSelection &curSelection);

private:
  void layoutSetup();
  // builds up tree item model with race list
  void updateItemModel(const SeasonData &season);
  // builds up model for race results
  void updateTableModel(int sessionId);
  // sets race view headers names after model rebuild
  void setItemHeaderData();

  SeasonData currentSeason;
  //================== Widgets ===================//
  QTreeView *treeView;
  QStandardItemModel *itemModel;
  QTableView *tableView;
  QSqlTableModel *tableModel;
  QLabel *seasonsLabel;
  QComboBox *seasonsCombo;
};

#endif// RESULTSWINDOW_H
