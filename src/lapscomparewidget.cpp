#include "lapscomparewidget.h"
#include <QtCore>
#include <QTableWidgetItem>

LapsCompareWidget::LapsCompareWidget(const QVector<LapsComp> &lapsData,
                                     QWidget *parent)
  : QTableWidget(parent)
{
  if (lapsData.isEmpty()) throw std::runtime_error("Invalid lap time data");
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Lap Times Compare");
  setWindowIcon(QIcon(":/better_icon.png"));
  setEditTriggers(NoEditTriggers);
  int maxLapsCount = 0;
  int initialRowCount = 200;/// todo bad!
  setRowCount(initialRowCount);
  setColumnCount(lapsData.size());
  for (int i = 0; i < lapsData.size(); ++i)
  {
    const auto lapsStrList = parseLaps(lapsData.at(i).laps);
    setHorizontalHeaderItem(i, new QTableWidgetItem(lapsData.at(i).driver));
    int lapRow = 0;
    for (const auto &lapStr : lapsStrList)
    {
      const auto itemStr = convertToLapTime(lapStr);
      auto item = new QTableWidgetItem(itemStr);
      if (isBestLap(lapStr, lapsData.at(i).bestLap))
      {
        QFont boldFont;
        boldFont.setBold(true);
        boldFont.setItalic(true);
        boldFont.setUnderline(true);
        item->setFont(boldFont);
      }
      setItem(lapRow++, i, item);
    }
    if (maxLapsCount < lapsStrList.size()) maxLapsCount = lapsStrList.size();
  }
  for (int i = 0; i < maxLapsCount; ++i)
    setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i + 1)));
  setRowCount(maxLapsCount);
  resizeColumnsToContents();
  resizeRowsToContents();
  resize(720, 480);/// todo smarter resize to contents
}

QStringList LapsCompareWidget::parseLaps(const QString &lapsString)
{
  if (lapsString.isEmpty()) return QStringList();
  return lapsString.split(", ");
}

bool LapsCompareWidget::isBestLap(const QString &lapTime,
                                  const QString &bestLap) const
{
  if (qFuzzyCompare(bestLap.toDouble(), 0)) return false;
  if (abs(bestLap.toDouble() - lapTime.toDouble()) < 0.002) return true;
  return false;
}

QString LapsCompareWidget::convertToLapTime(const QString &lapTime) const
{
  auto sections = lapTime.split('.');
  QString result;
  for (int i = 0; i < sections.size(); ++i)
  {
    if (i == 0)
    {
      int minutes = static_cast<int>(sections[0].toFloat() / 60.f);
      int seconds = static_cast<int>(fmodf(sections[0].toFloat(), 60.f));
      result.append(QString::number(minutes) + ':' + QString::number(seconds));
    }
    if (i == 1) result.append("." + sections[1]);
  }
  if (result == "0:0")// case for invalid lap time
    return "-:----";
  return result;
}
