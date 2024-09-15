#pragma once

#include <QString>
#include <QObject>
#include <QReadWriteLock>
#include <QPointer>
#include <QSet>
#include <QEventLoop>
#include <QTimer>
#include <QQueue>
#include <utils/settings.h>
#include "pluginspecification.h"

namespace ExtensionSystem
{
class PluginManager : public QObject
{
    Q_OBJECT
public:
    static PluginManager &instance();
    static QString platformName();
    QString pluginIID() const;
    void setPluginIID(const QString &newPluginIID);

    void addObject(QObject *obj);
    void removeObject(QObject *obj);
    QVector<QPointer<QObject>> allObjects();
    QReadWriteLock *listLock();

    void loadPlugins();
    const QVector<PluginSpecification *> loadQueue();
    Utils::Settings *settings() const;
    void setSettings(Utils::Settings *settings);

private:
    PluginManager();
    bool loadQueue(PluginSpecification *spec,
                   QVector<PluginSpecification *> &queue,
                   QVector<PluginSpecification *> &circularityCheckQueue);
    void loadPlugin(PluginSpecification *spec, PluginState destState);
    void startDelayedInitialize();
    QString m_pluginIID;
    Utils::Settings *m_settings;
    mutable QReadWriteLock m_lock;
    QVector<QPointer<QObject>> m_allObjects;
    QSet<PluginSpecification *> m_asynchronousPlugins;
    QVector<PluginSpecification *> m_pluginSpecs;
    QEventLoop *m_shutdownEventLoop = nullptr;
    QQueue<PluginSpecification *> m_delayedInitializeQueue;
    QTimer m_delayedInitializeTimer;
    bool m_isInitializationDone = false;
signals:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);
    void pluginsChanged();
    void initializationDone();
};
} // namespace ExtensionSystem
