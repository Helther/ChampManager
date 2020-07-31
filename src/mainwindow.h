#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <resultswindow.h>
#include <QDialog>
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

private:
  Ui::MainWindow *ui;
  QTabWidget *tabW;
  Resultswindow *resultsW;
  UserData *userData;
  //===================Menus & Actions=====================//
private slots:
  void on_addNewRace();
  void on_rmSeasonRes();

private:
  void initData();
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
  void removed();/// todo add connect for res widget update slot

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
  void addedRace();///todo connect to results widget update
private slots:
  void on_addSeason();
  void updateSeasonsCombo();

private:
  void accept() override;
  ChooseSeason *seasonW;
  QPushButton *addSeasonButton;
  //practice, quali and race setters
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


class AddSeason : public QDialog
{
  Q_OBJECT

public:
  explicit AddSeason(QWidget *parent = nullptr);

signals:
  void addedSeason();///todo connect to results widget update

private:
  void accept() override;
  QLineEdit *lineEdit;
  QDialogButtonBox *buttonBox;
};

//===========================User data============================//

// holds all meta data (currently only seasons info)
class UserData
{
public:
  UserData();

  void init();
  void addSeason(const QString &name);
  void updateSeasons();
  SeasonData getCurrentSeason() { return currentSeason; }
  QVector<SeasonData> getSeasons() { return seasons; }
  void setCurrentSeason(SeasonData season) { currentSeason = season; }

private:
  SeasonData currentSeason;
  QVector<SeasonData> seasons;
};

#endif// MAINWINDOW_H
