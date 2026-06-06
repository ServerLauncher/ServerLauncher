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

    const MetaIndex& index() const;
    const MetaPackage* package(const QString& uid) const;
    const MetaPlatform* findPlatform(const QString& uid) const;

    bool isIndexLoaded() const;
    bool isPackageLoaded(const QString& uid) const;

    MetaIndexCache* indexCache() const { return m_indexCache; }
    MetaPackageCache* packageCache(const QString& uid) const;

signals:
    void indexLoaded();
    void indexLoadedFromNetwork();
    void packageLoaded(const QString& uid);
    void loadFailed(const QString& error);

private:
    MetaIndexCache* m_indexCache;
    QHash<QString, MetaPackageCache*> m_packageCaches;
    QString m_cacheDir;
    QString m_url;
    QNetworkAccessManager* m_nam;
};