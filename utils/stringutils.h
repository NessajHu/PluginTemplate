#pragma once
#include <QString>
#include <QJsonValue>
namespace Utils {
bool readMultiLineString(const QJsonValue &value, QString &out);
}
