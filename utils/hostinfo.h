#pragma once
#include <qsystemdetection.h>
#include "utilsglobal.h"

namespace Utils
{

enum class OSType
{
    Windows,
    Linux,
    MacOS,
    OtherUnix,
    Other
};

class UTILS_EXPORT HostInfo
{
public:
    static constexpr OSType hostOsType()
    {
#ifdef Q_OS_WIN
        return OSType::Windows;
#elif defined(Q_OS_LINUX)
        return OSType::Linux;
#elif defined(Q_OS_MAC)
        return OSType::MacOS;
#elif defined(Q_OS_UNIX)
        return OSType::OtherUnix;
#else
        return OSType::Other;
#endif
    }

    static constexpr bool isWindowsHost()
    {
        return hostOsType() == OSType::Windows;
    }
    static constexpr bool isLinuxHost()
    {
        return hostOsType() == OSType::Linux;
    }
    static constexpr bool isMacHost()
    {
        return hostOsType() == OSType::MacOS;
    }
    static constexpr bool isOtherUnixHost()
    {
        return hostOsType() == OSType::OtherUnix;
    }
    static constexpr bool isUnixHost()
    {
        return isLinuxHost() || isMacHost() || isOtherUnixHost();
    }
    static constexpr bool isOtherHost()
    {
        return hostOsType() == OSType::Other;
    }
};
} // namespace Utils
