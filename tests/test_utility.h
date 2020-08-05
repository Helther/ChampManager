#ifndef TEST_UTILITY_H
#define TEST_UTILITY_H

#include <QtCore>

template<typename T>
void ASSERT_EQUAL(const QString &testName, const T &lhs, const T &rhs)
{
  QString lhsString;
  QString rhsString;
  QTextStream sl(&lhsString);
  QTextStream sr(&rhsString);
  sl << lhs;
  sr << rhs;
  if (lhs != rhs)
    throw std::runtime_error("assertion failed: " + testName.toStdString()
                             + "\n" + lhsString.toStdString() + " is not equal "
                             + rhsString.toStdString());
}

void TEST_BEGIN(const QString &name)
{
  qDebug() << "------------------- Running Test " << name << " ---------------";
}

void TEST_END()
{
  qDebug() << "--------------- Test Finished Successfuly ---------------";
}

#endif// TEST_UTILITY_H
