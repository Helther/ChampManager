#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <resultswindow.h>
#include <QDialog>
#include <appdata.h>
#include <QtDebug>/// todo debug
//forward decl
class UserData;
//
namespace Ui {
class MainWindow;
}
//=======================Main window=======================///
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  UserData *getUserData()
  {
    assert(userData != nullptr);///debug
    return userData;
  }
signals:
  void on_resultsChanged(const SeasonData &season);
public slots:
  void on_seasonsChanged(const SeasonData);


  //============Menus & Actions===========//
private slots:
  // called when new race added
  void on_addNewRace();
  // called when season was removed
  void on_rmSeasonRes();
  // about menu
  void about() {}/// todo define
  // license menu
  void license() {}/// todo define

private:
  // initializes user data
  void initData();
  // creates actions for menus
  void createActions();
  void createMenus();

  UserData *userData;
  //=========== Widgets ==============//
  Ui::MainWindow *ui;
  QTabWidget *tabW;
  Resultswindow *resultsW;
  QMenu *fileMenu;
  QMenu *helpMenu;
  QAction *newRaceAct;
  QAction *rmSeasonRacesAct;
  QAction *exitAct;
  QAction *licenseAct;
  QAction *aboutAct;
};


//forward decl for dialog
class QDialogButtonBox;
class QComboBox;
class QLabel;
class QLineEdit;
//

//====================== Choose Season ============================//
class ChooseSeason : public QWidget
{
  Q_OBJECT
public:
  ChooseSeason(const QVector<SeasonData> &seasons, QWidget *parent = nullptr);
  SeasonData getSeasonData();
  auto getSeasons() { return seasonsCopy; }
  void setSeasons(const QVector<SeasonData> &sData);

private:
  QVector<SeasonData> seasonsCopy;
  QLabel *seasonsLabel;
  QComboBox *seasonsCombo;
};


//====================Remove Season Dialog=========================//
class RmSeasonResDialog : public QDialog
{
  Q_OBJECT
public:
  RmSeasonResDialog(const QVector<SeasonData> &seasons,
                    QWidget *parent = nullptr);
public slots:
  void acceptedSg();
signals:
  void removed(const SeasonData &season);

private:
  ChooseSeason *seasonW;
  QDialogButtonBox *buttonBox;
};

//======================New Race Dialog===========================//
class NewRaceDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NewRaceDialog(const QVector<SeasonData> &seasons,
                         QWidget *parent = nullptr);
signals:
  void
    addedRace(const SeasonData &season);///todo connect to results widget update
private slots:
  void on_addSeason();
  void updateSeasonsCombo(const SeasonData &season);

private:
  void accept() override;
  void layoutSetup();
  //================== Widgets ===================//
  ChooseSeason *seasonW;
  QPushButton *addSeasonButton;
  //practice, quali and race setters
  QLabel *pLabel;
  QLabel *qLabel;
  QLabel *rLabel;
  QLineEdit *pFilePath;
  QLineEdit *qFilePath;
  QLineEdit *rFilePath;
  QPushButton *pBrowseButton;
  QPushButton *qBrowseButton;
  QPushButton *rBrowseButton;

  QDialogButtonBox *buttonBox;
};

//======================Add New Season Dialog===========================//
class AddSeason : public QDialog
{
  Q_OBJECT

public:
  explicit AddSeason(QWidget *parent = nullptr);

signals:
  void addedSeason(const SeasonData &season);

private:
  void accept() override;
  QLineEdit *lineEdit;
  QDialogButtonBox *buttonBox;
};

#endif// MAINWINDOW_H
