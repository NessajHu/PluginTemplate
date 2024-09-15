#pragma once
#include "utilsglobal.h"
#include <QSettings>

namespace Utils {

class UTILS_EXPORT Settings : private QSettings
{
public:
    using QSettings::setParent;
    template<typename T>
    void setValueWithDefault(const QString &key, const T &val, const T &defaultValue)
    {
        if (val == defaultValue)
            remove(key);
        else
            setValue(key, val);
    }

    template<typename T>
    void setValueWithDefault(const QString &key, const T &val)
    {
        if (val == T())
            remove(key);
        else
            setValue(key, val);
    }
};

} // namespace Utils
