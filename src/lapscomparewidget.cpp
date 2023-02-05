#include "lapscomparewidget.h"
#include <QtCore>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <QHeaderView>


LapsCompareWidget::LapsCompareWidget(const QVector<LapsComp> &lapsData,
                                     QWidget *parent)
  : QTableWidget(nullptr)  // no parent nedeed for dialog window
{
  if (lapsData.isEmpty()) throw std::runtime_error("Invalid lap time data");
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Lap Times Comparison");
  setWindowIcon(QIcon(":/better_icon.png"));
  setEditTriggers(NoEditTriggers);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  int maxLapsCount = 0;
  std::vector<QStringList> parsedLaps;
  setColumnCount(lapsData.size());
  for (int i = 0; i < lapsData.size(); ++i)  // find num of laps to display
  {
    parsedLaps.push_back(parseLaps(lapsData.at(i).laps));
    if (maxLapsCount < parsedLaps.back().size()) maxLapsCount = parsedLaps.back().size();
  }
  setRowCount(maxLapsCount);
  for (int i = 0; i < lapsData.size(); ++i)
  {
    setHorizontalHeaderItem(i, new QTableWidgetItem(lapsData.at(i).driver));
    int lapRow = 0;
    for (const auto &lapStr : parsedLaps.at(i))
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
  }
  for (int i = 0; i < maxLapsCount; ++i)
    setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i + 1)));
  setMaximumSize(parent->size());
}

QSize LapsCompareWidget::sizeHint() const
{
  const QSize padding{40, 40};
  const QSize maxContentSize = maximumSize() + padding;
  QSize contentSize;
  for(int i = 0; i < columnCount(); ++i)
    contentSize.setWidth(contentSize.width() + horizontalHeader()->sectionSize(i));
  for(int i = 0; i < rowCount(); ++i)
    contentSize.setHeight(contentSize.height() + verticalHeader()->sectionSize(i));
  return contentSize.width() > maxContentSize.width() ||
      contentSize.height() > maxContentSize.height() ?
      maximumSize() : contentSize + padding;
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
  if (result == "0:0")  // case for invalid lap time
    return "-:----";
  return result;
}
