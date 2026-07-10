#include "MetaManager.hpp"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

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

NetRequestTask* MetaManager::download(const QString& uid, const QString& mc_version,
                                      const QString& build, const QString& destDir) {
    const MetaVersion* ver = version(uid, mc_version);
    if (!ver) {
        qWarning() << "downloadBuild: version not loaded:" << uid << mc_version;
        emit downloadFailed(uid, mc_version, build, "Version metadata not loaded");
        return nullptr;
    }
 
    const MetaBuilds* buildInfo = nullptr;
    QString resolvedBuild = build;
 
    if (build.isEmpty()) {
        if (ver->builds.isEmpty()) {
            qWarning() << "downloadBuild: version has no builds:" << uid << mc_version;
            emit downloadFailed(uid, mc_version, build, "Version has no builds available");
            return nullptr;
        }
        buildInfo = &ver->builds.first();
        resolvedBuild = buildInfo->build;
    } else {
        buildInfo = ver->findByBuild(build);
        if (!buildInfo) {
            qWarning() << "downloadBuild: build not found:" << uid << mc_version << build;
            emit downloadFailed(uid, mc_version, build, "Build not found in version metadata");
            return nullptr;
        }
    }
 
    const QString key = uid + "/" + mc_version + "/" + resolvedBuild;
 
    if (m_downloadTasks.contains(key)) {
        auto existing = m_downloadTasks.value(key);
        if (!existing->isFinished())
            return existing;
    }
 
    if (buildInfo->download.url.isEmpty()) {
        emit downloadFailed(uid, mc_version, resolvedBuild, "Build has no download URL");
        return nullptr;
    }
 
    const QString baseDir = destDir.isEmpty()
        ? QDir(m_cacheDir).filePath(uid + "/" + mc_version + "/builds")
        : destDir;
 
    const QString fileName = buildInfo->download.name.isEmpty()
        ? QFileInfo(QUrl(buildInfo->download.url).path()).fileName()
        : buildInfo->download.name;
 
    if (fileName.isEmpty()) {
        emit downloadFailed(uid, mc_version, resolvedBuild, "Cannot determine destination file name");
        return nullptr;
    }
 
    const QString destPath = QDir(baseDir).filePath(fileName);
 
    auto request = std::make_shared<NetRequest>();
    request->url = QUrl(buildInfo->download.url);
 
    auto sink = std::make_unique<FileSink>(destPath);
 
    int validatorsAdded = 0;
 
    if (!buildInfo->download.sha256.isEmpty()) {
        sink->addValidator(std::make_unique<HashValidator>(
            QCryptographicHash::Sha256, buildInfo->download.sha256.toLower()));
        ++validatorsAdded;
    }
    if (!buildInfo->download.sha1.isEmpty()) {
        sink->addValidator(std::make_unique<HashValidator>(
            QCryptographicHash::Sha1, buildInfo->download.sha1.toLower()));
        ++validatorsAdded;
    }
    if (!buildInfo->download.md5.isEmpty()) {
        sink->addValidator(std::make_unique<HashValidator>(
            QCryptographicHash::Md5, buildInfo->download.md5.toLower()));
        ++validatorsAdded;
    }
 
    if (validatorsAdded == 0) {
        for (const QString& hash : { buildInfo->download.sha256,
                                      buildInfo->download.sha1,
                                      buildInfo->download.md5 }) {
            if (auto validator = HashValidator::fromExpectedHex(hash)) {
                sink->addValidator(std::move(validator));
                ++validatorsAdded;
            }
        }
    }
 
    if (validatorsAdded == 0) {
        qWarning() << "downloadBuild: no usable hash for" << key
                   << "- file integrity will NOT be verified";
    }
 
    request->sink = std::move(sink);
 
    auto task = new NetRequestTask(request, m_nam, this);
 
    connect(task, &Task::progress, this,
        [this, uid, mc_version, resolvedBuild](qint64 current, qint64 total, const QString&) {
        emit downloadProgress(uid, mc_version, resolvedBuild, current, total);
    });
 
    connect(task, &Task::completed, this, [this, uid, mc_version, resolvedBuild, key, destPath, task]() {
        m_downloadTasks.remove(key);
        emit downloaded(uid, mc_version, resolvedBuild, destPath);
        task->deleteLater();
    });
 
    connect(task, &Task::failed, this, [this, uid, mc_version, resolvedBuild, key, task](const QString& error) {
        m_downloadTasks.remove(key);
        emit downloadFailed(uid, mc_version, resolvedBuild, error);
        task->deleteLater();
    });
 
    connect(task, &Task::aborted, this, [this, key, task]() {
        m_downloadTasks.remove(key);
        task->deleteLater();
    });
 
    m_downloadTasks[key] = task;
    return task;
}

MetaPackageCache* MetaManager::packageCache(const QString& uid) const {
    return m_packageCaches.value(uid, nullptr);
}

MetaVersionCache* MetaManager::versionCache(const QString& uid, const QString& mc_version) const {
    return m_versionCaches.value(uid + "/" + mc_version, nullptr);
}