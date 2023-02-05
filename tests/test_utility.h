#pragma once

#include <QtCore>

template<typename T>
void ASSERT_EQUAL(const QString testName, const T &lhs, const T &rhs)
{
  QString lhsString;
  QString rhsString;
  QTextStream sl(&lhsString);
  QTextStream sr(&rhsString);
  sl << lhs;
  sr << rhs;
  if (lhs != rhs)
    throw std::runtime_error("Assertion Failed: " + testName.toStdString()
                               + "\n" + lhsString.toStdString() + " is not equal "
                               + rhsString.toStdString());
  //assert(lhs == rhs && QString("Assertion Failed: %1\n%2 is not equal %3").arg(testName)
  //                           .arg(lhsString).arg(rhsString).toStdString().c_str());
}

void ASSERT_FAIL(const QString msg)
{
  throw std::runtime_error("Assertion Failed: " + msg.toStdString());
  //assert(false && QString("Assertion Failed: " + msg).toStdString().c_str());
}

void TEST_BEGIN(const QString name)
{
  qDebug() << "------------------- Running Test " << name << " ---------------";
}

void TEST_END()
{
  qDebug() << "--------------- Test Finished Successfuly ---------------";
}

