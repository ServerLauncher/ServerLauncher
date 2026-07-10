#pragma once
#include <QObject>
#include <QHash>
#include <QNetworkAccessManager>
#include <QString>
#include "MetaCache.hpp"
#include "LoadMetaTask.hpp"
#include "net/FileSink.hpp"
#include "net/NetRequestTask.hpp"
#include "net/NetRequest.hpp"
#include "net/HashValidator.hpp"
#include <QCryptographicHash>

class MetaManager : public QObject {
    Q_OBJECT
public:
    explicit MetaManager(const QString& cacheDir,
                         const QString& url,
                         QNetworkAccessManager* nam,
                         QObject* parent = nullptr);

    void init();

    LoadMetaTask* loadIndex();
    LoadMetaTask* loadPackage(const QString& uid);
    LoadMetaTask* loadVersion(const QString& uid, const QString& mc_version);

    const MetaIndex& index() const;
    const MetaPackage* package(const QString& uid) const;
    const MetaVersion* version(const QString& uid, const QString& mc_version) const;
    const MetaPlatform* findPlatform(const QString& uid) const;

    bool isIndexLoaded() const;
    bool isPackageLoaded(const QString& uid) const;
    bool isVersionLoaded(const QString& uid, const QString& mc_version) const;

    MetaIndexCache* indexCache() const { return m_indexCache; }
    MetaPackageCache* packageCache(const QString& uid) const;
    MetaVersionCache* versionCache(const QString& uid, const QString& mc_version) const;

    NetRequestTask* download(const QString& uid, const QString& mc_version, 
                                const QString& build = QString(), const QString& destDir = QString());

signals:
    void indexLoaded();
    void indexLoadedFromNetwork();
    void packageLoaded(const QString& uid);
    void packageLoadedFromNetwork(const QString& uid);
    void versionLoaded(const QString& uid, const QString& mc_version);
    void versionLoadedFromNetwork(const QString& uid, const QString& mc_version);
    void loadFailed(const QString& error);

    void downloadProgress(const QString& uid, const QString& mc_version,
                            const QString& build, qint64 current, qint64 total);
    void downloaded(const QString& uid, const QString& mc_version,
                    const QString& build, const QString& filePath);
    void downloadFailed(const QString& uid, const QString& mc_version,
                        const QString& build, const QString& error);

private:
    LoadMetaTask* startOrReuseTask(
        QHash<QString, LoadMetaTask*> MetaManager::* taskMapPtr,
        const QString& key,
        MetaCache* cache,
        const QString& url,
        std::function<void()> onSuccess);
    
    MetaIndexCache* m_indexCache;
    QHash<QString, MetaPackageCache*> m_packageCaches;
    QHash<QString, MetaVersionCache*> m_versionCaches;
    
    QHash<QString, LoadMetaTask*> m_indexTasks;
    QHash<QString, LoadMetaTask*> m_packageTasks;
    QHash<QString, LoadMetaTask*> m_versionTasks;
    QHash<QString, NetRequestTask*> m_downloadTasks;
    
    QString m_cacheDir;
    QString m_url;
    QNetworkAccessManager* m_nam;
};