#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <resultswindow.h>
#include <QDialog>
#include <QtDebug>///debug
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

  UserData *getUserData();

private:
  Ui::MainWindow *ui;
  QTabWidget *tabW;
  resultswindow *resultsW;
  UserData *userData;
  //===================Menus & Actions=====================//
private slots:
  void on_addNewRace();
  void on_rmSeasonRes();

private:
  void createActions();
  void createMenus();

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
struct SeasonData
{
  int id;
  QString name;
  ///add other info like teams and drivers lists
};
Q_DECLARE_METATYPE(SeasonData)

//====================== Choose Season ============================//
class ChooseSeason : public QWidget
{
  Q_OBJECT
public:
  ChooseSeason(const QVector<SeasonData> &seasons, QWidget *parent = nullptr);
  SeasonData getSeasonData();

private:
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
  void accept() override;

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

private:
  ChooseSeason *seasonW;
  //practise, quali and race setters
  QLabel *pLabel;
  QLineEdit *pFilePath;
  QPushButton *pBrowseButton;
  QLabel *qLabel;
  QLineEdit *qFilePath;
  QPushButton *qBrowseButton;
  QLabel *rLabel;
  QLineEdit *rFilePath;
  QPushButton *rBrowseButton;

  QDialogButtonBox *buttonBox;
};

//===========================User data============================//

// holds all meta data (currently only seasons info)
class UserData
{
public:
  UserData() {}
  void init() { seasons.push_back({ 1, QString("testSeason") }); }///todo temp
  void addSeason(const QString &name);
  void removeSeason(SeasonData season);
  SeasonData getCurrentSeason() { return currentSeason; }
  QVector<SeasonData> getSeasons() { return seasons; }
  void setCurrentSeason(SeasonData season);

private:
  SeasonData currentSeason;
  QVector<SeasonData> seasons;
};

#endif// MAINWINDOW_H
