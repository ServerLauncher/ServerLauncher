#include "MetaManager.hpp"
#include <QDebug>

MetaManager::MetaManager(const QString& cacheDir,
                         const QString& url,
                         QNetworkAccessManager* nam,
                         QObject* parent)
    : QObject(parent)
    , m_cache(new MetaCache(cacheDir, this))
    , m_url(url)
    , m_nam(nam)
{ }

void MetaManager::init() {
    QString errorMessage;
    if (!m_cache->loadFromDisk(errorMessage)) {
        qWarning() << "Failed to load meta from disk:" << errorMessage;
    }
}

LoadMetaTask* MetaManager::load() {
    auto task = new LoadMetaTask(m_cache, m_url, m_nam, this);
    connect(task, &Task::completed, this, [this](){
        emit indexUpdated();
    });
    connect(task, &Task::failed, this, [this](const QString& error){
        emit loadFailed(error);
    });

    return task;
}

const MetaIndex& MetaManager::index() const {
    return m_cache->index();
}

const MetaPlatform* MetaManager::findPlatform(const QString& uid) const {
    return m_cache->index().findPlatformbyUid(uid);
}

bool MetaManager::isLoaded() const {
    return m_cache->isLoaded();
}