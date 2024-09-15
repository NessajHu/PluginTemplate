#include "settings.h"

namespace Utils {


void Settings::beginGroup(const QString &prefix)
{
    QSettings::beginGroup(prefix);
}

QVariant Settings::value(const QString &key) const
{
    return QSettings::value(key);
}

QVariant Settings::value(const QString &key, const QVariant &def) const
{
    return QSettings::value(key, def);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
}

void Settings::remove(const QString &key)
{
    QSettings::remove(key);
}

bool Settings::contains(const QString &key) const
{
    return QSettings::contains(key);
}

QStringList Settings::childKeys() const
{
    return QSettings::childKeys();
}

} // namespace Utils

