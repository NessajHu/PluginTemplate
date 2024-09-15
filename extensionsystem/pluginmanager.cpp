#include "pluginmanager.h"

#include "extensionsystemtr.h"
#include "pluginspecification.h"
#include <utils/algorithm.h>
#include <utils/hostinfo.h>


namespace ExtensionSystem
{
constexpr int kDelayedInitializeInterval = 20;
PluginManager::PluginManager() {}

bool PluginManager::loadQueue(PluginSpecification *spec, QVector<PluginSpecification *> &queue, QVector<PluginSpecification *> &circularityCheckQueue)
{
    if (queue.contains(spec))
        return true;
    // check for circular dependencies
    if (circularityCheckQueue.contains(spec)) {
        spec->m_errorString = Tr::tr("Circular dependency detected:");
        spec->m_errorString.value() += QLatin1Char('\n');
        int index = circularityCheckQueue.indexOf(spec);
        for (int i = index; i < circularityCheckQueue.size(); ++i) {
            const PluginSpecification *depSpec = circularityCheckQueue.at(i);
            spec->m_errorString.value().append(Tr::tr("%1 (%2) depends on")
                                            .arg(depSpec->name(), depSpec->version()));
            spec->m_errorString.value() += QLatin1Char('\n');
        }
        spec->m_errorString.value().append(Tr::tr("%1 (%2)").arg(spec->name(), spec->version()));
        return false;
    }
    circularityCheckQueue.append(spec);
    // check if we have the dependencies
    if (spec->state() == PluginState::Invalid || spec->state() == PluginState::Read) {
        queue.append(spec);
        return false;
    }

    // add dependencies
    const QHash<PluginDependency, PluginSpecification *> deps = spec->dependencySpecifications();
    for (auto it = deps.cbegin(), end = deps.cend(); it != end; ++it) {
        // Skip test dependencies since they are not real dependencies but just force-loaded
        // plugins when running tests
        if (it.key().type == PluginDependency::Type::Test)
            continue;
        PluginSpecification *depSpec = it.value();
        if (!loadQueue(depSpec, queue, circularityCheckQueue)) {
            spec->m_errorString =
                Tr::tr("Cannot load plugin because dependency failed to load: %1 (%2)\nReason: %3")
                    .arg(depSpec->name(), depSpec->version(), depSpec->errorString().value_or(""));
            return false;
        }
    }
    // add self
    queue.append(spec);
    return true;
}

static inline QString getPlatformName()
{
    if (Utils::HostInfo::isMacHost())
        return QLatin1String("OS X");
    else if (Utils::HostInfo::isUnixHost())
        return QLatin1String(Utils::HostInfo::isLinuxHost() ? "Linux" : "Unix");
    else if (Utils::HostInfo::isWindowsHost())
        return QLatin1String("Windows");
    return QLatin1String("Unknown");
}

QString PluginManager::platformName()
{
    static const QString result = getPlatformName() + " (" + QSysInfo::prettyProductName() + ')';
    return result;
}

void PluginManager::loadPlugin(PluginSpecification *spec, PluginState destState)
{
    if (spec->hasError() || spec->state() != destState-1)
        return;

    // don't load disabled plugins.
    if (!spec->isEffectivelyEnabled() && destState == PluginState::Loaded)
        return;


    const std::string specName = spec->name().toStdString();

    switch (destState) {
    case PluginState::Running: {
        spec->initializeExtensions();
        return;
    }
    case PluginState::Deleted:
        spec->kill();
        return;
    default:
        break;
    }
    // check if dependencies have loaded without error
    const QHash<PluginDependency, PluginSpecification *> deps = spec->dependencySpecifications();
    for (auto it = deps.cbegin(), end = deps.cend(); it != end; ++it) {
        if (it.key().type != PluginDependency::Type::Required)
            continue;
        PluginSpecification *depSpec = it.value();
        if (depSpec->state() != destState) {
            spec->m_errorString =
                Tr::tr("Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
                    .arg(depSpec->name(), depSpec->version(), depSpec->errorString().value_or(""));
            return;
        }
    }
    switch (destState) {
    case PluginState::Loaded: {
        spec->loadLibrary();
        break;
    }
    case PluginState::Initialized: {
        spec->initializePlugin();
        break;
    }
    case PluginState::Stopped:
        if (spec->stop() == PluginShutdownFlag::AsynchronousShutdown) {
            m_asynchronousPlugins << spec;
            connect(spec->plugin(), &IPlugin::asynchronousShutdownFinished, this, [this, spec] {
                m_asynchronousPlugins.remove(spec);
                if (m_asynchronousPlugins.isEmpty())
                    m_shutdownEventLoop->exit();
            });
        }
        break;
    default:
        break;
    }
}

void PluginManager::startDelayedInitialize()
{
    {
        while (!m_delayedInitializeQueue.empty()) {
            PluginSpecification *spec = m_delayedInitializeQueue.front();
            const std::string specName = spec->name().toStdString();
            m_delayedInitializeQueue.dequeue();
            bool delay = spec->delayedInitialize();
            if (delay)
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        m_isInitializationDone = true;
    }
    emit initializationDone();
}

Utils::Settings *PluginManager::settings() const
{
    return m_settings;
}

void PluginManager::setSettings(Utils::Settings *settings)
{

    if (m_settings)
        delete m_settings;
    m_settings = settings;
    if (m_settings)
        m_settings->setParent(this);
}

PluginManager &PluginManager::instance()
{
    static PluginManager m_instance;
    return m_instance;
}

QString PluginManager::pluginIID() const
{
    return m_pluginIID;
}

void PluginManager::setPluginIID(const QString &newPluginIID)
{
    m_pluginIID = newPluginIID;
}

void PluginManager::addObject(QObject *obj)
{
    {
        QWriteLocker lock(&m_lock);
        if (obj == nullptr) {
            return;
        }
        if (m_allObjects.contains(obj)) {
            return;
        }


        m_allObjects.append(obj);
    }
    emit objectAdded(obj);
}

void PluginManager::removeObject(QObject *obj)
{

    if (obj == nullptr) {
        return;
    }

    if (!m_allObjects.contains(obj)) {
        return;
    }

    emit aboutToRemoveObject(obj);
    QWriteLocker lock(&m_lock);
    m_allObjects.removeAll(obj);
}

QVector<QPointer<QObject> > PluginManager::allObjects()
{
    return m_allObjects;
}

QReadWriteLock *PluginManager::listLock()
{
    return &m_lock;
}

void PluginManager::loadPlugins()
{

    const QVector<PluginSpecification *> queue = loadQueue();

    for (PluginSpecification *spec : queue)
        loadPlugin(spec, PluginState::Initialized);

    {
        Utils::reverseForeach(queue, [this](PluginSpecification *spec) {
            loadPlugin(spec, PluginState::Running);
            if (spec->state() == PluginState::Running) {
                m_delayedInitializeQueue.enqueue(spec);
            } else {
                // Plugin initialization failed, so cleanup after it
                spec->kill();
            }
        });
    }
    emit pluginsChanged();

    m_delayedInitializeTimer.setInterval(kDelayedInitializeInterval);
    m_delayedInitializeTimer.setSingleShot(true);
    connect(&m_delayedInitializeTimer,
            &QTimer::timeout,
            this,
            &PluginManager::startDelayedInitialize);
    m_delayedInitializeTimer.start();
}

const QVector<PluginSpecification *> PluginManager::loadQueue()
{
    QVector<PluginSpecification *> queue;
    for (PluginSpecification *spec : std::as_const(m_pluginSpecifications)) {
        QVector<PluginSpecification *> circularityCheckQueue;
        loadQueue(spec, queue, circularityCheckQueue);
    }
    return queue;
}
} // namespace ExtensionSystem
