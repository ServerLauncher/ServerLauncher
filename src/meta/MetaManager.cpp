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
            m_cacheDir, platform.uid, this);
        
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

LoadMetaTask* MetaManager::startOrReuseTask(
    QHash<QString, LoadMetaTask*> MetaManager::* taskMapPtr,
    const QString& key,
    MetaCache* cache,
    const QString& url,
    std::function<void()> onSuccess)
{
    auto& taskMap = this->*taskMapPtr;

    if (taskMap.contains(key)) {
        auto existing = taskMap.value(key);
        if (!existing->isFinished()) {
            return existing;
        }
    }

    auto task = new LoadMetaTask(cache, url, m_nam, this);

    connect(task, &Task::completed, this, [this, taskMapPtr, key, onSuccess, task]() {
        (this->*taskMapPtr).remove(key);
        onSuccess();
        task->deleteLater();
    });
    connect(task, &Task::failed, this, [this, taskMapPtr, key, task](const QString& error) {
        (this->*taskMapPtr).remove(key);
        emit loadFailed(error);
        task->deleteLater();
    });
    connect(task, &Task::aborted, this, [this, taskMapPtr, key, task]() {
        (this->*taskMapPtr).remove(key);
        task->deleteLater();
    });

    taskMap[key] = task;
    return task;
}

LoadMetaTask* MetaManager::loadIndex() {
    const QString key = QStringLiteral("index");
    return startOrReuseTask(
        &MetaManager::m_indexTasks, key,
        m_indexCache, m_url + "index.json",
        [this]() { emit indexLoadedFromNetwork(); });
}

LoadMetaTask* MetaManager::loadPackage(const QString& uid) {
    if (!m_packageCaches.contains(uid)) {
        auto cache = new MetaPackageCache(m_cacheDir, uid, this);

        connect(cache, &MetaPackageCache::packageUpdated,
                this, [this, uid]() {
            emit packageLoaded(uid);
        });

        m_packageCaches[uid] = cache;
    }

    const MetaPlatform* platform = m_indexCache->index().findPlatformByUid(uid);
    if (!platform) {
        qWarning() << "loadPackage: uid not found in index, falling back to default URL:" << uid;
    }

    const QString url = platform
        ? m_url + platform->url
        : m_url + "packages/" + uid + ".json";

    if (platform && !platform->sha256.isEmpty()) {
       m_packageCaches[uid]->setSha256(platform->sha256);
    }

    return startOrReuseTask(
        &MetaManager::m_packageTasks, uid,
        m_packageCaches[uid], url,
        [this, uid]() { emit packageLoadedFromNetwork(uid); });
}

LoadMetaTask* MetaManager::loadVersion(const QString& uid, const QString& mc_version) {
    const QString key = uid + "/" + mc_version;

    if (!m_versionCaches.contains(key)) {
        auto cache = new MetaVersionCache(
            m_cacheDir, uid, mc_version, this);

        connect(cache, &MetaVersionCache::versionUpdated,
                this, [this, uid, mc_version](){
            emit versionLoaded(uid, mc_version);
        });

        m_versionCaches[key] = cache;

        QString err;
        cache->loadFromDisk(err);
    }

    const MetaPackage* pkg = package(uid);
    if (pkg) {
        const MetaBuild* build = pkg->findByMcVersion(mc_version);
        if (build && !build->sha256.isEmpty())
            m_versionCaches[key]->setSha256(build->sha256);
    }

    const QString url = m_url + uid + "/" + mc_version + ".json";

    return startOrReuseTask(
        &MetaManager::m_versionTasks, key,
        m_versionCaches[key], url,
        [this, uid, mc_version]() { emit versionLoadedFromNetwork(uid, mc_version); });
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

const MetaVersion* MetaManager::version(const QString& uid, const QString& mc_version) const {
    const QString key = uid + "/" + mc_version;
    auto it = m_versionCaches.find(key);
    if (it == m_versionCaches.end())
        return nullptr;
    return it.value()->version();
}

const MetaPlatform* MetaManager::findPlatform(const QString& uid) const {
    return m_indexCache->index().findPlatformByUid(uid);
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

bool MetaManager::isVersionLoaded(const QString& uid, const QString& mc_version) const {
    const QString key = uid + "/" + mc_version;
    auto it = m_versionCaches.find(key);
    if (it == m_versionCaches.end())
        return false;
    return it.value()->isLoaded();
}

MetaPackageCache* MetaManager::packageCache(const QString& uid) const {
    return m_packageCaches.value(uid, nullptr);
}

MetaVersionCache* MetaManager::versionCache(const QString& uid, const QString& mc_version) const {
    return m_versionCaches.value(uid + "/" + mc_version, nullptr);
}