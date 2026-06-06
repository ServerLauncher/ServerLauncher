#include "MetaManager.hpp"
#include <QDebug>
#include <QDir>

MetaManager::MetaManager(const QString& cacheDir,
                         const QString& url,
                         QNetworkAccessManager* nam,
                         QObject* parent)
    : QObject(parent)
    , m_indexCache(new MetaIndexCache(cacheDir, this))
    , m_cacheDir(cacheDir)
    , m_url(url)
    , m_nam(nam)
{
    connect(m_indexCache, &MetaIndexCache::indexUpdated,
            this, &MetaManager::indexLoaded);
}

void MetaManager::init() {
    QString errorMessage;
    if (!m_indexCache->loadFromDisk(errorMessage)) {
        qWarning() << "Failed to load index from disk:" << errorMessage;
    }

    for (const auto& platform : m_indexCache->index().platforms) {
        auto cache = new MetaPackageCache(
            QDir(m_cacheDir).filePath("packages"), platform.uid, this);
        
        if (!platform.sha256.isEmpty()) {
            cache->setSha256(platform.sha256);
        }

        m_packageCaches[platform.uid] = cache;

        connect(cache, &MetaPackageCache::packageUpdated,
                this, [this, uid = platform.uid]() {
            emit packageLoaded(uid);
        });

        QString err;
        cache->loadFromDisk(err);
    }
}

LoadMetaTask* MetaManager::loadIndex() {
    auto task = new LoadMetaTask(m_indexCache, m_url + "index.json", m_nam, this);

    connect(task, &Task::completed, this, [this]() {
        emit indexLoadedFromNetwork();
    });
    connect(task, &Task::failed, this, [this](const QString& error) {
        emit loadFailed(error);
    });

    return task;
}

LoadMetaTask* MetaManager::loadPackage(const QString& uid) {
    if (!m_packageCaches.contains(uid)) {
        auto cache = new MetaPackageCache(
            QDir(m_cacheDir).filePath("packages"), uid, this);

        connect(cache, &MetaPackageCache::packageUpdated,
                this, [this, uid]() {
            emit packageLoaded(uid);
        });

        m_packageCaches[uid] = cache;
    }

    const MetaPlatform* platform = m_indexCache->index().findPlatformbyUid(uid);
    const QString url = platform
        ? m_url + platform->url
        : m_url + "packages/" + uid + ".json";

    if (platform && !platform->sha256.isEmpty()) {
       m_packageCaches[uid]->setSha256(platform->sha256);
    }

    auto task = new LoadMetaTask(m_packageCaches[uid], url, m_nam, this);

    connect(task, &Task::failed, this, [this](const QString& error) {
        emit loadFailed(error);
    });

    return task;
}

const MetaIndex& MetaManager::index() const {
    return m_indexCache->index();
}

const MetaPackage* MetaManager::package(const QString& uid) const {
    auto it = m_packageCaches.find(uid);
    if (it == m_packageCaches.end())
        return nullptr;
    return it.value()->package();
}

const MetaPlatform* MetaManager::findPlatform(const QString& uid) const {
    return m_indexCache->index().findPlatformbyUid(uid);
}

bool MetaManager::isIndexLoaded() const {
    return m_indexCache->isLoaded();
}

bool MetaManager::isPackageLoaded(const QString& uid) const {
    auto it = m_packageCaches.find(uid);
    if (it == m_packageCaches.end())
        return false;
    return it.value()->isLoaded();
}

MetaPackageCache* MetaManager::packageCache(const QString& uid) const {
    return m_packageCaches.value(uid, nullptr);
}