#include "pluginspecification.h"
#include <QFileInfo>
#include <QHashFunctions>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>
#include <utils/hostinfo.h>
#include <utils/stringutils.h>
#include "pluginmanager.h"
#include "extensionsystemtr.h"
#include "iplugin.h"


Q_LOGGING_CATEGORY(pluginLog, "qtc.extensionsystem", QtWarningMsg)

namespace ExtensionSystem {

namespace Constants
{
const char kPluginMetadata[] = "MetaData";
const char kPluginName[] = "Name";
const char kPluginVersion[] = "Version";
const char kPluginCompatversion[] = "CompatVersion";
const char kPluginRequired[] = "Required";
const char kPluginExperimental[] = "Experimental";
const char kPluginDisabledByDefault[] = "DisabledByDefault";
const char kVendor[] = "Vendor";
const char kCopyright[] = "Copyright";
const char kLicense[] = "License";
const char kDescription[] = "Description";
const char kLongDescription[] = "LongDescription";
const char kUrl[] = "Url";
const char kCategory[] = "Category";
const char kPlatform[] = "Platform";
const char kDependencies[] = "Dependencies";
const char kDependencyName[] = "Name";
const char kDependencyVersion[] = "Version";
const char kDependencyType[] = "Type";
const char kDependencyTypeSoft[] = "optional";
const char kDependencyTypeHard[] = "required";
const char kDependencyTypeTest[] = "test";
const char kArguments[] = "Arguments";
const char kArgumentName[] = "Name";
const char kArgumentParameter[] = "Parameter";
const char kArgumentDescription[] = "Description";
const char versionRegExp[] = "^([0-9]+)(?:[.]([0-9]+))?(?:[.]([0-9]+))?(?:_([0-9]+))?$";
}
namespace Helpers
{
static inline QString msgValueMissing(const char *key)
{
    return Tr::tr("\"%1\" is missing").arg(QLatin1String(key));
}

static inline QString msgValueIsNotAString(const char *key)
{
    return Tr::tr("Value for key \"%1\" is not a string")
        .arg(QLatin1String(key));
}

static inline QString msgValueIsNotABool(const char *key)
{
    return Tr::tr("Value for key \"%1\" is not a bool")
        .arg(QLatin1String(key));
}

static inline QString msgValueIsNotAObjectArray(const char *key)
{
    return Tr::tr("Value for key \"%1\" is not an array of objects")
        .arg(QLatin1String(key));
}

static inline QString msgValueIsNotAMultilineString(const char *key)
{
    return Tr::tr("Value for key \"%1\" is not a string and not an array of strings")
        .arg(QLatin1String(key));
}

static inline QString msgInvalidFormat(const char *key, const QString &content)
{
    return Tr::tr("Value \"%2\" for key \"%1\" has invalid format")
        .arg(QLatin1String(key), content);
}

static inline bool isValidVersion(const QString &version)
{
    return QRegularExpression(Constants::versionRegExp).match(version).hasMatch();
}

}


QString PluginSpecification::name() const
{
    return m_name;
}

QString PluginSpecification::version() const
{
    return m_version;
}

QString PluginSpecification::compatVersion() const
{
    return m_compatVersion;
}

QString PluginSpecification::vendor() const
{
    return m_vendor;
}

QString PluginSpecification::category() const
{
    return m_category;
}

QString PluginSpecification::description() const
{
    return m_description;
}

QString PluginSpecification::longDescription() const
{
    return m_longDescription;
}

QString PluginSpecification::url() const
{
    return m_url;
}

QString PluginSpecification::revision() const
{
    return m_revision;
}

QString PluginSpecification::location() const
{
    return m_location;
}

QString PluginSpecification::copyright() const
{
    return m_copyright;
}

QString PluginSpecification::license() const
{
    return m_license;
}

QRegularExpression PluginSpecification::platformSpecificationRegExp() const
{
    return m_platformSpecification;
}

IPlugin *PluginSpecification::plugin() const
{
    return m_plugin;
}

bool PluginSpecification::isRequired() const
{
    return m_required;
}

bool PluginSpecification::isExperimental() const
{
    return m_experimental;
}

bool PluginSpecification::isEnabledByDefault() const
{
    return m_enabledByDefault;
}

bool PluginSpecification::isEnabledBySettings() const
{
    return m_enabledBySettings;
}

QJsonObject PluginSpecification::metaData() const
{
    return m_metaData;
}

PluginState PluginSpecification::state() const
{
    return m_state;
}

QVector<PluginDependency> PluginSpecification::dependencies() const
{
    return m_dependencies;
}

QHash<PluginDependency, PluginSpecification *> PluginSpecification::dependencySpecifications() const
{
    return m_dependencySpecifications;
}

QStringList PluginSpecification::arguments() const
{
    return m_arguments;
}

QVector<PluginArgumentDescription> PluginSpecification::argumentDescriptions() const
{
    return m_argumentDescriptions;
}

bool PluginSpecification::initializeExtensions()
{
    if (m_errorString)
        return false;
    if (m_state != PluginState::Initialized) {
        if (m_state == PluginState::Running)
            return true;
        m_errorString = ::ExtensionSystem::Tr::tr(
            "Cannot perform extensionsInitialized because state != Initialized");
        return false;
    }
    if (!m_plugin) {
        m_errorString = ::ExtensionSystem::Tr::tr(
            "Internal error: have no plugin instance to perform extensionsInitialized");
        return false;
    }
    m_plugin->extensionsInitialized();
    m_state = PluginState::Running;
    return true;
}

void PluginSpecification::addArguments(const QStringList &arguments)
{
    m_arguments.append(arguments);
}

bool PluginSpecification::read(const QString &filePath)
{
    reset();
    QFileInfo fileInfo(filePath);
    m_location = fileInfo.absolutePath();
    m_filePath = fileInfo.absoluteFilePath();
    m_loader.emplace();
    if (Utils::HostInfo::isMacHost())
        m_loader->setLoadHints(QLibrary::ExportExternalSymbolsHint);
    m_loader->setFileName(filePath);
    if (m_loader->fileName().isEmpty()) {
        return false;
    }

    if (!readMetaData(m_loader->metaData()))
        return false;

    m_state = PluginState::Read;
    return true;
}

void PluginSpecification::reset()
{
    m_name.clear();
    m_version.clear();
    m_compatVersion.clear();
    m_vendor.clear();
    m_category.clear();
    m_description.clear();
    m_longDescription.clear();
    m_url.clear();
    m_revision.clear();
    m_location.clear();
    m_filePath.clear();
    m_platformSpecification.setPattern(QString());
    m_copyright.clear();
    m_license.clear();
    m_plugin = nullptr;
    m_required = false;
    m_experimental = false;
    m_enabledByDefault = true;
    m_metaData = QJsonObject();
    m_state = PluginState::Invalid;
    m_dependencies.clear();
    m_dependencySpecifications.clear();
    m_arguments.clear();
    m_argumentDescriptions.clear();
    m_loader.reset();
    m_errorString.reset();
    m_staticPlugin.reset();
}

bool PluginSpecification::loadLibrary()
{
    if (m_errorString.has_value())
        return false;
    if (m_state != PluginState::Resolved) {
        if (m_state == PluginState::Loaded)
            return true;
        m_errorString =
            ::ExtensionSystem::Tr::tr("Loading the library failed because state != Resolved");
        return false;
    }
    if (m_loader && !m_loader->load()) {
        m_errorString = QDir::toNativeSeparators(m_filePath) + QString::fromLatin1(": ")
                      + m_loader->errorString();
        return false;
    }
    auto *pluginObject = m_loader ? qobject_cast<IPlugin *>(m_loader->instance())
                                : qobject_cast<IPlugin *>(m_staticPlugin->instance());
    if (!pluginObject) {
        m_errorString =
            ::ExtensionSystem::Tr::tr("Plugin is not valid (does not derive from IPlugin)");
        if (m_loader)
            m_loader->unload();
        return false;
    }
    m_state = PluginState::Loaded;
    m_plugin = pluginObject;
    return true;
}

bool PluginSpecification::readMetaData(const QJsonObject &pluginMetaData)
{
    qCDebug(pluginLog) << "MetaData:" << QJsonDocument(pluginMetaData).toJson();
    QJsonValue value;
    value = pluginMetaData.value(QLatin1String("IID"));
    if (!value.isString()) {
        qCDebug(pluginLog) << "Not a plugin (no string IID found)";
        return false;
    }
    if (value.toString() != PluginManager::instance().pluginIID()) {
        qCDebug(pluginLog) << "Plugin ignored (IID does not match)";
        return false;
    }

    value = pluginMetaData.value(QLatin1String(Constants::kPluginMetadata));
    if (!value.isObject()) {
        return reportError(::ExtensionSystem::Tr::tr("Plugin meta data not found"));
    }
    m_metaData = value.toObject();

    value = m_metaData.value(QLatin1String(Constants::kPluginName));
    if (value.isUndefined())
        return reportError(Helpers::msgValueMissing(Constants::kPluginName));
    if (!value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kPluginName));
    m_name = value.toString();

    value = m_metaData.value(QLatin1String(Constants::kPluginVersion));
    if (value.isUndefined())
        return reportError(Helpers::msgValueMissing(Constants::kPluginVersion));
    if (!value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kPluginVersion));
    m_version = value.toString();
    if (!Helpers::isValidVersion(m_version))
        return reportError(Helpers::msgInvalidFormat(Constants::kPluginVersion, m_version));

    value = m_metaData.value(QLatin1String(Constants::kPluginCompatversion));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kPluginCompatversion));
    m_compatVersion = value.toString(m_version);
    if (!value.isUndefined() && !Helpers::isValidVersion(m_compatVersion))
        return reportError(Helpers::msgInvalidFormat(Constants::kPluginCompatversion, m_compatVersion));

    value = m_metaData.value(QLatin1String(Constants::kPluginRequired));
    if (!value.isUndefined() && !value.isBool())
        return reportError(Helpers::msgValueIsNotABool(Constants::kPluginRequired));
    m_required = value.toBool(false);
    qCDebug(pluginLog) << "required = " << m_required;

    value = m_metaData.value(QLatin1String(Constants::kPluginExperimental));
    if (!value.isUndefined() && !value.isBool())
        return reportError(Helpers::msgValueIsNotABool(Constants::kPluginExperimental));
    m_experimental = value.toBool(false);
    qCDebug(pluginLog) << "experimental = " << m_experimental;

    value = m_metaData.value(QLatin1String(Constants::kPluginDisabledByDefault));
    if (!value.isUndefined() && !value.isBool())
        return reportError(Helpers::msgValueIsNotABool(Constants::kPluginDisabledByDefault));
    m_enabledByDefault = !value.toBool(false);
    qCDebug(pluginLog) << "enabledByDefault = " << m_enabledByDefault;

    if (m_experimental)
        m_enabledByDefault = false;
    m_enabledBySettings = m_enabledByDefault;

    value = m_metaData.value(QLatin1String(Constants::kVendor));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kVendor));
    m_vendor = value.toString();

    value = m_metaData.value(QLatin1String(Constants::kCopyright));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kCopyright));
    m_copyright = value.toString();

    value = m_metaData.value(QLatin1String(Constants::kDescription));
    if (!value.isUndefined() && !Utils::readMultiLineString(value, m_description))
        return reportError(Helpers::msgValueIsNotAString(Constants::kDescription));

    value = m_metaData.value(QLatin1String(Constants::kLongDescription));
    if (!value.isUndefined() && !Utils::readMultiLineString(value, m_longDescription))
        return reportError(Helpers::msgValueIsNotAString(Constants::kLongDescription));

    value = m_metaData.value(QLatin1String(Constants::kUrl));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kUrl));
    m_url = value.toString();

    value = m_metaData.value(QLatin1String(Constants::kCategory));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kCategory));
    m_category = value.toString();

    value = m_metaData.value(QLatin1String(Constants::kLicense));
    if (!value.isUndefined() && !Utils::readMultiLineString(value, m_license))
        return reportError(Helpers::msgValueIsNotAMultilineString(Constants::kLicense));

    value = m_metaData.value(QLatin1String(Constants::kPlatform));
    if (!value.isUndefined() && !value.isString())
        return reportError(Helpers::msgValueIsNotAString(Constants::kPlatform));
    const QString platformSpec = value.toString().trimmed();
    if (!platformSpec.isEmpty()) {
        m_platformSpecification.setPattern(platformSpec);
        if (!m_platformSpecification.isValid()) {
            return reportError(::ExtensionSystem::Tr::tr("Invalid platform specification \"%1\": %2")
                                   .arg(platformSpec, m_platformSpecification.errorString()));
        }
    }

    value = m_metaData.value(QLatin1String(Constants::kDependencies));
    if (!value.isUndefined() && !value.isArray())
        return reportError(Helpers::msgValueIsNotAObjectArray(Constants::kDependencies));
    if (!value.isUndefined()) {
        const QJsonArray array = value.toArray();
        for (const QJsonValue &v : array) {
            if (!v.isObject())
                return reportError(Helpers::msgValueIsNotAObjectArray(Constants::kDependencies));
            QJsonObject dependencyObject = v.toObject();
            PluginDependency dep;
            value = dependencyObject.value(QLatin1String(Constants::kDependencyName));
            if (value.isUndefined()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Dependency: %1")
                        .arg(Helpers::msgValueMissing(Constants::kDependencyName)));
            }
            if (!value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Dependency: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kDependencyName)));
            }
            dep.name = value.toString();
            value = dependencyObject.value(QLatin1String(Constants::kDependencyVersion));
            if (!value.isUndefined() && !value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Dependency: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kDependencyVersion)));
            }
            dep.version = value.toString();
            if (!Helpers::isValidVersion(dep.version)) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Dependency: %1")
                        .arg(Helpers::msgInvalidFormat(Constants::kDependencyVersion, dep.version)));
            }
            dep.type = PluginDependency::Type::Required;
            value = dependencyObject.value(QLatin1String(Constants::kDependencyType));
            if (!value.isUndefined() && !value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Dependency: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kDependencyType)));
            }
            if (!value.isUndefined()) {
                const QString typeValue = value.toString();
                if (typeValue.toLower() == QLatin1String(Constants::kDependencyTypeHard)) {
                    dep.type = PluginDependency::Type::Required;
                } else if (typeValue.toLower() == QLatin1String(Constants::kDependencyTypeSoft)) {
                    dep.type = PluginDependency::Type::Optional;
                } else if (typeValue.toLower() == QLatin1String(Constants::kDependencyTypeTest)) {
                    dep.type = PluginDependency::Type::Test;
                } else {
                    return reportError(
                        ::ExtensionSystem::Tr::tr(
                            "Dependency: \"%1\" must be \"%2\" or \"%3\" (is \"%4\").")
                            .arg(QLatin1String(Constants::kDependencyType),
                                 QLatin1String(Constants::kDependencyTypeHard),
                                 QLatin1String(Constants::kDependencyTypeSoft),
                                 typeValue));
                }
            }
            m_dependencies.append(dep);
        }
    }

    value = m_metaData.value(QLatin1String(Constants::kArguments));
    if (!value.isUndefined() && !value.isArray())
        return reportError(Helpers::msgValueIsNotAObjectArray(Constants::kArguments));
    if (!value.isUndefined()) {
        const QJsonArray array = value.toArray();
        for (const QJsonValue &v : array) {
            if (!v.isObject())
                return reportError(Helpers::msgValueIsNotAObjectArray(Constants::kArguments));
            QJsonObject argumentObject = v.toObject();
            PluginArgumentDescription arg;
            value = argumentObject.value(QLatin1String(Constants::kArgumentName));
            if (value.isUndefined()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Argument: %1")
                        .arg(Helpers::msgValueMissing(Constants::kArgumentName)));
            }
            if (!value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Argument: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kArgumentName)));
            }
            arg.name = value.toString();
            if (arg.name.isEmpty()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Argument: \"%1\" is empty")
                        .arg(QLatin1String(Constants::kArgumentName)));
            }
            value = argumentObject.value(QLatin1String(Constants::kArgumentDescription));
            if (!value.isUndefined() && !value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Argument: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kArgumentDescription)));
            }
            arg.description = value.toString();
            value = argumentObject.value(QLatin1String(Constants::kArgumentParameter));
            if (!value.isUndefined() && !value.isString()) {
                return reportError(
                    ::ExtensionSystem::Tr::tr("Argument: %1")
                        .arg(Helpers::msgValueIsNotAString(Constants::kArgumentParameter)));
            }
            arg.parameter = value.toString();
            m_argumentDescriptions.append(arg);
            qCDebug(pluginLog) << "Argument:" << arg.name << "Parameter:" << arg.parameter
                               << "Description:" << arg.description;
        }
    }

    return true;
}

bool PluginSpecification::reportError(const QString &errorString)
{
    m_errorString = errorString;
    return true;
}

std::optional<QString> PluginSpecification::errorString() const
{
    return m_errorString;
}

bool PluginSpecification::hasError() const
{
    return m_errorString.has_value();
}


bool PluginSpecification::isAvailableForHostPlatform() const
{
    return m_platformSpecification.pattern().isEmpty()
           || m_platformSpecification.match(PluginManager::platformName()).hasMatch();
}

bool PluginSpecification::isEffectivelyEnabled() const
{

    if (!isAvailableForHostPlatform())
        return false;
    return isEnabledBySettings();
}

void PluginSpecification::kill()
{
    if (!m_plugin)
        return;
    delete m_plugin;
    m_plugin = nullptr;
    m_state = PluginState::Deleted;
}

bool PluginSpecification::initializePlugin()
{
    if (m_errorString.has_value())
        return false;
    if (m_state != PluginState::Loaded) {
        if (m_state == PluginState::Initialized)
            return true;
        m_errorString = ::ExtensionSystem::Tr::tr(
            "Initializing the plugin failed because state != Loaded");
        return false;
    }
    if (!m_plugin) {
        m_errorString = ::ExtensionSystem::Tr::tr(
            "Internal error: have no plugin instance to initialize");
        return false;
    }
    QString err;
    if (!m_plugin->initialize(m_arguments, err)) {
        m_errorString = ::ExtensionSystem::Tr::tr("Plugin initialization failed: %1").arg(err);
        return false;
    }
    m_state = PluginState::Initialized;
    return true;
}

PluginShutdownFlag PluginSpecification::stop()
{
    if (!m_plugin)
        return PluginShutdownFlag::SynchronousShutdown;
    m_state = PluginState::Stopped;
    return m_plugin->aboutToShutdown();
}

bool PluginSpecification::delayedInitialize()
{
    if (m_errorString.has_value())
        return false;
    if (m_state != PluginState::Running)
        return false;
    if (!m_plugin) {
        m_errorString = ::ExtensionSystem::Tr::tr(
            "Internal error: have no plugin instance to perform delayedInitialize");
        return false;
    }
    const bool res = m_plugin->delayedInitialize();
    return res;
}

bool PluginDependency::operator==(const PluginDependency &other) const
{
    return name == other.name && version == other.version && type == other.type;
}

size_t qHash(const PluginDependency &value)
{
    return qHash(value.name);
}
} // namespace ExtensionSystem
