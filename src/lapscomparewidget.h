#ifndef LAPSCOMPAREWIDGET_H
#define LAPSCOMPAREWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QDebug>

// used to initialize a model for viewing lap comparisons
struct LapsComp
{
  QString driver;
  QString laps;
  QString bestLap;
};

class LapsCompareWidget : public QTableWidget
{
  Q_OBJECT

public:
  LapsCompareWidget(const QVector<LapsComp> &lapsData, QWidget *parent = nullptr);

private:
  QStringList parseLaps(const QString &lapsString);

  bool isBestLap(const QString& lapTime, const QString& bestLap) const;
  // convets time string from x.xxx to x:xx.xxx
  QString convertToLapTime(const QString &lapTime) const;

};

#endif// LAPSCOMPAREWIDGET_h
