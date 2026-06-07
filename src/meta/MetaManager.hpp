#pragma once
#include <QObject>
#include <QHash>
#include <QNetworkAccessManager>
#include <QString>
#include "MetaCache.hpp"
#include "LoadMetaTask.hpp"

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

signals:
    void indexLoaded();
    void indexLoadedFromNetwork();
    void packageLoaded(const QString& uid);
    void packageLoadedFromNetwork(const QString& uid);
    void versionLoaded(const QString& uid, const QString& mc_version);
    void versionLoadedfromNetwork(const QString& uid, const QString& mc_version);
    void loadFailed(const QString& error);

private:
    MetaIndexCache* m_indexCache;
    QHash<QString, MetaPackageCache*> m_packageCaches;
    QHash<QString, MetaVersionCache*> m_versionCaches;
    QString m_cacheDir;
    QString m_url;
    QNetworkAccessManager* m_nam;
};