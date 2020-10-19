#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QDialog>
#include <QPushButton>
#include <resultswindow.h>
#include <appdata.h>
#include <parser.h>
#include <dbhelper.h>

static constexpr QEvent::Type GUI_NEW_RACE_SUCCESS_EVENT =
  static_cast<QEvent::Type>(QEvent::User + 1);
static constexpr QEvent::Type GUI_NEW_RACE_FAIL_EVENT =
  static_cast<QEvent::Type>(QEvent::User + 2);
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
  struct SessionData
  {
    FileType type;
    RaceLogInfo data;
  };

  class GuiAddRaceSuccessEvent : public QEvent
  {
  public:
    GuiAddRaceSuccessEvent(const SeasonData &Season)
      : QEvent(GUI_NEW_RACE_SUCCESS_EVENT), season(Season)
    {}
    SeasonData season;
  };

  class GuiAddRaceFailEvent : public QEvent
  {
  public:
    GuiAddRaceFailEvent(std::exception_ptr Except)
      : QEvent(GUI_NEW_RACE_FAIL_EVENT), except(Except)
    {}
    std::exception_ptr except;
  };

  class NewSessionDBThread : public QThread
  {
  public:
    NewSessionDBThread(MainWindow *MainW,
                       const SeasonData &Season,
                       const SessionData &PLog,
                       const SessionData &QLog,
                       const SessionData &RLog)
      : mainW(MainW), season(Season), pLog(PLog), qLog(QLog), rLog(RLog)
    {}

  private:
    void run() override;
    MainWindow *mainW;
    const SeasonData season;
    std::exception_ptr except;
    const SessionData pLog;
    const SessionData qLog;
    const SessionData rLog;
  };

  template<class Parser> class SessionParserThread : public QThread
  {
  public:
    SessionParserThread(const QString &FilePath) : filePath(FilePath) {}
    std::exception_ptr except;
    RaceLogInfo SessionData;
    FileType type;

  private:
    void run() override
    {
      try
      {
        auto file = QFile(filePath);
        auto parser = Parser(file);
        SessionData = parser.getParseData();
        type = parser.getFileType();
      } catch (...)
      {
        except = std::current_exception();
      }
    }
    const QString filePath;
  };


public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  inline UserData *getUserData()
  {
#ifdef QT_DEBUG
    assert(userData != nullptr);
#endif
    return userData;
  }
signals:
  void dbReseted();
  void resultsChanged(const SeasonData &season);
public slots:
  void on_seasonsChanged(const SeasonData);
  // called when needed to add new race
  void on_newRaceAccepted(const SeasonData &season,
                          const QString &pFile,
                          const QString &qFile,
                          const QString &rFile);

  //============Menus & Actions===========//
private slots:

  // called when open new race dialog
  void on_addNewRace();
  // called when season was removed
  void on_rmSeasonRes();
  // about menu
  void about();
  // license menu
  void license();
  // deletes all tables in db and crates empty ones
  void resetBdData();

private:
  // initializes user data
  void initData();
  // creates actions for menus
  void createActions();
  void createMenus();
  bool event(QEvent *event) override;


  UserData *userData;
  //=========== Widgets ==============//
  Ui::MainWindow *ui;
  QTabWidget *tabW;
  Resultswindow *resultsW;
  QMenu *fileMenu;
  QMenu *helpMenu;
  QAction *newRaceAct;
  QAction *rmSeasonRacesAct;
  QAction *resetAllDataAct;
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
  inline SeasonData getSeasonData()
  {
    return seasonsCombo->currentData().value<SeasonData>();
  }
  inline auto getSeasons() { return seasonsCopy; }
  // repopulates season combo box and local array
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
  void addedRace(const SeasonData &season,
                 const QString &pFile,
                 const QString &qFile,
                 const QString &rFile);
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


//====================== Add New Season Dialog =====================//
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


//===================== Browse button =========================//
class BrowseButton : public QPushButton
{
  Q_OBJECT

public:
  BrowseButton(QLineEdit *pathLine,
               const QString &fileFilter = QString(),
               QWidget *parent = nullptr);

private slots:
  void chooseFile();

private:
  QLineEdit *pathLinePtr;
  QString filter;
};

#endif// MAINWINDOW_H
