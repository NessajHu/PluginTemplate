#pragma once
#include <QString>
#include <QVector>
#include <QStringList>
#include <QHash>
#include <optional>
#include <QPluginLoader>
#include <QJsonObject>
#include "extensionsystemglobal.h"


namespace ExtensionSystem {
enum PluginState
{
    Invalid,
    Read,
    Resolved,
    Loaded,
    Initialized,
    Running,
    Stopped,
    Deleted
};

struct PluginDependency
{
    enum class Type {
        Required,
        Optional,
        Test
    };

    PluginDependency() : type(Type::Required) {}

    friend size_t qHash(const PluginDependency &value);

    QString name;
    QString version;
    Type type;
    bool operator==(const PluginDependency &other) const;
    QString toString() const;
};

struct EXTENSIONSYSTEM_EXPORT PluginArgumentDescription
{
    QString name;
    QString parameter;
    QString description;
};

class IPlugin;
class EXTENSIONSYSTEM_EXPORT PluginSpecification
{
public:
    QString name() const;
    QString version() const;
    QString compatVersion() const;
    QString vendor() const;
    QString category() const;
    QString description() const;
    QString longDescription() const;
    QString url() const;
    QString revision() const;
    QString location() const;
    QString copyright() const;
    QString license() const;
    QRegularExpression platformSpecificationRegExp() const;
    IPlugin *plugin() const;
    bool isRequired() const;
    bool isExperimental() const;
    bool isEnabledByDefault() const;
    bool isEnabledBySettings() const;
    QJsonObject metaData() const;
    PluginState state() const;
    QVector<PluginDependency> dependencies() const;
    QHash<PluginDependency, PluginSpecification *> dependencySpecifications() const;
    QStringList arguments() const;
    QVector<PluginArgumentDescription> argumentDescriptions() const;

    bool initializeExtensions();

    void addArguments(const QStringList &arguments);
    bool read(const QString &filePath);
    void reset();
    bool loadLibrary();

    std::optional<QString> errorString() const;
    bool hasError() const;

    bool isAvailableForHostPlatform() const;
    bool isEffectivelyEnabled() const;
    void kill();
    bool initializePlugin();
    IPlugin::ShutdownFlag stop();
private:
    friend class PluginManager;
    bool readMetaData(const QJsonObject &pluginMetaData);
    bool reportError(const QString &errorString);
    QString m_name;
    QString m_version;
    QString m_compatVersion;
    QString m_vendor;
    QString m_category;
    QString m_description;
    QString m_longDescription;
    QString m_url;
    QString m_revision;
    QString m_location;
    QString m_filePath;
    QString m_copyright;
    QString m_license;
    QRegularExpression m_platformSpecification;
    IPlugin *m_plugin;
    bool m_required = false;
    bool m_experimental = false;
    bool m_enabledByDefault = true;
    bool m_enabledBySettings = true;
    QJsonObject m_metaData;
    PluginState m_state;
    QVector<PluginDependency> m_dependencies;
    QHash<PluginDependency, PluginSpecification *> m_dependencySpecifications;
    QStringList m_arguments;
    QVector<PluginArgumentDescription> m_argumentDescriptions;
    std::optional<QPluginLoader> m_loader;
    std::optional<QString> m_errorString;

    std::optional<QStaticPlugin> m_staticPlugin;
};

} // namespace ExtensionSystem
