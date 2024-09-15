#pragma once

#include <QObject>
#include "extensionsystemglobal.h"
namespace ExtensionSystem
{
enum class PluginShutdownFlag
{
    SynchronousShutdown,
    AsynchronousShutdown
};

class EXTENSIONSYSTEM_EXPORT IPlugin : public QObject
{
public:
    IPlugin();
    ~IPlugin() override;
    virtual bool initialize(const QStringList &arguments, QString &errorString);
    virtual void extensionsInitialized();
    virtual bool delayedInitialize();
    virtual PluginShutdownFlag aboutToShutdown();
protected:
    virtual void initialize();
signals:
    void asynchronousShutdownFinished();
};

} // namespace ExtensionSystem
