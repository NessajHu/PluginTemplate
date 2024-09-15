#include "stringutils.h"
#include <QJsonArray>
namespace Utils {
bool readMultiLineString(const QJsonValue &value, QString &out)
{
    if (value.isString()) {
        out = value.toString();
    } else if (value.isArray()) {
        QJsonArray array = value.toArray();
        QStringList lines;
        for (const QJsonValue &v : array) {
            if (!v.isString())
                return false;
            lines.append(v.toString());
        }
        out = lines.join(QLatin1Char('\n'));
    } else {
        return false;
    }
    return true;
}
} // namespace Utils
