#include "iplugin.h"

namespace ExtensionSystem
{
IPlugin::IPlugin()
{

}

bool IPlugin::initialize(const QStringList &arguments, QString &errorString)
{
    initialize();
    return true;
}

void IPlugin::extensionsInitialized()
{

}

bool IPlugin::delayedInitialize()
{
    return false;
}

PluginShutdownFlag IPlugin::aboutToShutdown()
{
    return PluginShutdownFlag::SynchronousShutdown;
}

void IPlugin::initialize()
{

}
} // namespace ExtensionSystem
